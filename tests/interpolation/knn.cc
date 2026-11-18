// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/utils/MD5.h"

#include "mir/method/knn/pick/Pick.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/search/PointSearch.h"

#include "interpolation.h"


namespace mir::interpolation::tests {


using method::knn::pick::Pick;
using method::knn::pick::PickFactory;

using P         = std::unique_ptr<const Pick>;
using Neighbors = std::vector<search::PointSearch::PointValueType>;
using Indices   = std::set<size_t>;


constexpr double MISSING = 9999.;


/*
 * 3x3 points, indexed (from the north-west):
 *   0 (10, 0)   1 (10, 10)   2 (10, 20)
 *   3 ( 0, 0)   4 ( 0, 10)   5 ( 0, 20)
 *   6 (-10, 0)  7 (-10, 10)  8 (-10, 20)
 */
const std::string GRID_3X3 = "{grid: [10, 10], area: [10, 0, -10, 20]}";


// chord distances: 10 degrees (~1.11e6m) between neighbours, ~1.56e6m along the diagonals
constexpr double RADIUS_NEIGHBOURS = 1.2e6;


PointXYZ point3(double lat, double lon) {
    return repres::Iterator::point_3D({lat, lon});
}


Indices payloads(const Neighbors& closest) {
    Indices idx;
    for (const auto& c : closest) {
        idx.insert(c.payload());
    }
    return idx;
}


struct Picker {
    explicit Picker(const std::string& gridspec = GRID_3X3) : repres(representation(gridspec)) {
        param.set("point-search-trees", "memory");
        tree = std::make_unique<search::PointSearch>(param, *repres);
    }

    Neighbors pick(const std::string& name, double lat, double lon) {
        P p(PickFactory::build(name, param));
        p->distance(*repres);

        Neighbors closest;
        p->pick(*tree, point3(lat, lon), closest);
        return closest;
    }

    param::SimpleParametrisation param;
    repres::RepresentationHandle repres;
    std::unique_ptr<search::PointSearch> tree;
};


CASE("neighbour picking methods") {
    SECTION("nclosest") {
        Picker picker;
        picker.param.set("nclosest", 4L);

        auto closest = picker.pick("nclosest", 1., 1.);
        EXPECT(payloads(closest) == Indices({0, 3, 4, 6}));
        EXPECT_EQUAL(closest.front().payload(), 3);
    }


    SECTION("nearest-neighbour-with-lowest-index") {
        Picker picker;

        picker.param.set("nclosest", 1L);
        EXPECT(payloads(picker.pick("nearest-neighbour-with-lowest-index", 1., 1.)) == Indices({3}));

        // equidistant points 3 and 4, the lowest index is picked
        picker.param.set("nclosest", 4L);
        EXPECT(payloads(picker.pick("nearest-neighbour-with-lowest-index", 0., 5.)) == Indices({3}));
        EXPECT(payloads(picker.pick("nearest-neighbour-with-lowest-index", 0., 6.)) == Indices({4}));

        // comparison, by nclosest
        P a(PickFactory::build("nearest-neighbour-with-lowest-index", picker.param));
        EXPECT(a->sameAs(*P(PickFactory::build("nearest-neighbour-with-lowest-index", picker.param))));

        picker.param.set("nclosest", 1L);
        EXPECT_NOT(a->sameAs(*P(PickFactory::build("nearest-neighbour-with-lowest-index", picker.param))));
    }


    SECTION("distance") {
        Picker picker;

        picker.param.set("distance", RADIUS_NEIGHBOURS);
        EXPECT(payloads(picker.pick("distance", 0., 0.)) == Indices({0, 3, 4, 6}));

        picker.param.set("distance", 1e5);
        EXPECT(payloads(picker.pick("distance", 0., 0.)) == Indices({3}));
        EXPECT(picker.pick("distance", 5., 5.).empty());
    }


    SECTION("distance-and-nclosest") {
        Picker picker;
        picker.param.set("nclosest", 2L);

        picker.param.set("distance", RADIUS_NEIGHBOURS);
        auto closest = picker.pick("distance-and-nclosest", 0., 0.);
        EXPECT_EQUAL(closest.size(), 2);
        EXPECT_EQUAL(closest.front().payload(), 3);

        picker.param.set("distance", 1e5);
        EXPECT(payloads(picker.pick("distance-and-nclosest", 0., 0.)) == Indices({3}));
    }


    SECTION("distance-or-nclosest") {
        Picker picker;
        picker.param.set("nclosest", 2L);

        picker.param.set("distance", RADIUS_NEIGHBOURS);
        EXPECT(payloads(picker.pick("distance-or-nclosest", 0., 0.)) == Indices({0, 3, 4, 6}));

        picker.param.set("distance", 1e5);
        auto closest = picker.pick("distance-or-nclosest", 0., 0.);
        EXPECT_EQUAL(closest.size(), 2);
        EXPECT_EQUAL(closest.front().payload(), 3);
    }


    SECTION("sample, sorted-sample") {
        Picker picker;
        picker.param.set("distance", RADIUS_NEIGHBOURS);
        const Indices within{0, 3, 4, 6};

        // all, if not more than nclosest
        picker.param.set("nclosest", 8L);
        EXPECT(payloads(picker.pick("sample", 0., 0.)) == within);

        // a sample of nclosest, sorted by distance or not
        picker.param.set("nclosest", 2L);
        for (const auto& name : std::vector<std::string>{"sample", "sorted-sample"}) {
            auto sample = payloads(picker.pick(name, 0., 0.));
            EXPECT_EQUAL(sample.size(), 2);
            EXPECT(std::includes(within.begin(), within.end(), sample.begin(), sample.end()));
        }

        picker.param.set("nclosest", 3L);
        auto sorted = picker.pick("sorted-sample", 1., 1.);
        auto p      = point3(1., 1.);
        EXPECT(std::is_sorted(sorted.begin(), sorted.end(), [&p](const auto& a, const auto& b) {
            return PointXYZ::distance2(a.point(), p) < PointXYZ::distance2(b.point(), p);
        }));

        // comparison, by nclosest and distance
        P x(PickFactory::build("sample", picker.param));
        EXPECT(x->sameAs(*P(PickFactory::build("sample", picker.param))));

        picker.param.set("nclosest", 2L);
        EXPECT_NOT(x->sameAs(*P(PickFactory::build("sample", picker.param))));
        picker.param.set("nclosest", 3L);

        // comparison, by the sample
        P a(PickFactory::build("sorted-sample", picker.param));
        P b(PickFactory::build("sorted-sample", picker.param));
        P s(PickFactory::build("sample", picker.param));
        EXPECT(a->sameAs(*b));
        EXPECT_NOT(a->sameAs(*s));

        picker.param.set("distance", 2. * RADIUS_NEIGHBOURS);
        EXPECT_NOT(a->sameAs(*P(PickFactory::build("sorted-sample", picker.param))));
    }


    SECTION("nclosest-or-nearest (k)") {
        for (const auto& name : std::vector<std::string>{"nclosest-or-nearest", "k"}) {
            Picker picker;

            picker.param.set("nclosest", 1L);
            EXPECT(payloads(picker.pick(name, 1., 1.)) == Indices({3}));

            // equidistant points are all picked
            EXPECT(payloads(picker.pick(name, 0., 5.)) == Indices({3, 4}));

            picker.param.set("nclosest", 4L);
            EXPECT(payloads(picker.pick(name, 1., 1.)) == Indices({0, 3, 4, 6}));
        }

        // coincident points (the pole)
        Picker poles("{grid: [90, 90]}");
        poles.param.set("nclosest", 1L);
        EXPECT(payloads(poles.pick("nclosest-or-nearest", 90., 0.)) == Indices({0, 1, 2, 3}));
    }


    SECTION("longest-element-diagonal-and-nclosest") {
        Picker picker;
        picker.param.set("nclosest", 2L);

        P p(PickFactory::build("longest-element-diagonal-and-nclosest", picker.param));
        EXPECT_EQUAL(p->n(), 2);
        p->distance(*picker.repres);

        // the search switches between radius and nclosest, giving the same
        for (size_t i = 0; i < 3; ++i) {
            Neighbors closest;
            p->pick(*picker.tree, point3(0., 0.), closest);
            EXPECT(closest.size() <= 2);
            EXPECT(payloads(closest).count(3) == 1);
        }
    }


    SECTION("number of neighbours") {
        param::SimpleParametrisation param;
        param.set("nclosest", 3L);
        param.set("distance", 1000.);

        struct test_t {
            std::string name;
            size_t n;
        };

        for (const auto& test : std::vector<test_t>{
                 {"nclosest", 3},
                 {"nearest-neighbour-with-lowest-index", 1},
                 {"distance", 4},
                 {"distance-and-nclosest", 3},
                 {"distance-or-nclosest", 3},
                 {"sample", 3},
                 {"sorted-sample", 3},
                 {"nclosest-or-nearest", 3},
             }) {
            EXPECT_EQUAL(P(PickFactory::build(test.name, param))->n(), test.n);
        }
    }


    SECTION("unknown") {
        param::SimpleParametrisation param;
        EXPECT_THROWS_AS(P(PickFactory::build("unknown", param)), exception::SeriousBug);
    }
}


CASE("neighbour picking methods, as method options") {
    std::set<std::string> digests;

    for (const auto& name :
         std::vector<std::string>{"nclosest", "nearest-neighbour-with-lowest-index", "distance",
                                  "distance-and-nclosest", "distance-or-nclosest", "sample", "sorted-sample",
                                  "nclosest-or-nearest", "longest-element-diagonal-and-nclosest"}) {
        const std::string spec = "{type: k-nearest, nearest-method: " + name + ", nclosest: 3, distance: 1000}";

        auto m = make(spec);
        EXPECT(m->json_str().find(R"("nearest-method":")" + name + "\"") != std::string::npos);
        EXPECT(m->json_str().find(R"("nclosest":3)") != std::string::npos ||
               m->json_str().find(R"("distance":1000)") != std::string::npos);

        // the spec builds the same method
        EXPECT(*make(m->json_str()) == *m);

        std::ostringstream str;
        str << *m;
        EXPECT(str.str().find("nearestMethod=") != std::string::npos);

        EXPECT(m->sameAs(*make(spec)));

        auto other = make("{type: k-nearest, nearest-method: " + name + ", nclosest: 2, distance: 2000}");
        EXPECT_NOT(m->sameAs(*other));

        // the hash differs by neighbour picking method
        eckit::MD5 h;
        m->hash(h);
        EXPECT(digests.insert(h.digest()).second);
    }
}


CASE("k-nearest neighbours methods") {
    auto in = representation(GRID_3X3);

    // values by index
    MIRValuesVector values(in->numberOfPoints());
    for (size_t i = 0; i < values.size(); ++i) {
        values[i] = static_cast<double>(i);
    }

    auto input = field(in, values);


    SECTION("nearest-neighbour: input values at the input points") {
        auto out = representation("{grid: [10, 10], area: [10, 0, 0, 10]}");

        for (const auto& spec : std::vector<std::string>{"nn", "k-nearest"}) {
            auto result = interpolate(*make(spec), input, *out);
            EXPECT(result.values(0) == MIRValuesVector({0., 1., 3., 4.}));
        }
    }


    SECTION("nearest-neighbour: input values elsewhere") {
        auto out    = representation("{grid: [1, 1], area: [10, 0, -10, 20]}");
        auto result = interpolate(*make("nn"), input, *out);

        for (const auto& v : result.values(0)) {
            EXPECT(std::find(values.begin(), values.end(), v) != values.end());
        }
    }


    SECTION("k-nearest: neighbours average") {
        auto out    = representation("{grid: [10, 10], area: [5, 5, -5, 15]}");
        auto result = interpolate(
            *make("{type: k-nearest, nearest-method: nclosest, nclosest: 4, distance-weighting: 'no'}"), input, *out);

        EXPECT(result.values(0) == MIRValuesVector({2., 3., 5., 6.}));
    }


    SECTION("k-nearest-statistics: neighbours statistics") {
        auto out = representation("{grid: [10, 10], area: [5, 5, -5, 15]}");

        auto maximum = interpolate(*make("k-nearest-statistics"), input, *out);
        EXPECT(maximum.values(0) == MIRValuesVector({4., 5., 7., 8.}));

        auto minimum =
            interpolate(*make("{type: k-nearest-statistics, interpolation-statistics: minimum}"), input, *out);
        EXPECT(minimum.values(0) == MIRValuesVector({0., 1., 3., 4.}));

        auto m = make("k-nearest-statistics");
        EXPECT_NOT(m->sameAs(*make("{type: k-nearest-statistics, nclosest: 2}")));
        EXPECT_NOT(m->sameAs(*make("k-nearest")));

        // the statistics, in the spec
        auto n = make("{type: k-nearest-statistics, interpolation-statistics: minimum}");
        EXPECT(n->json_str().find(R"("interpolation-statistics":"minimum")") != std::string::npos);
        EXPECT(*make(n->json_str()) == *n);
        EXPECT(*m != *n);
        EXPECT_NOT(m->sameAs(*n));
    }


    SECTION("output points outside the input domain are missing") {
        auto out    = representation("{grid: [30, 30]}");
        auto result = interpolate(*make("nn"), input, *out);

        // only (0, 0) is inside the input domain
        EXPECT(result.hasMissing());
        EXPECT_EQUAL(count_missing(result), out->numberOfPoints() - 1);
    }


    SECTION("no output points inside the input domain") {
        auto out = representation("{grid: [10, 10], area: [60, 100, 50, 110]}");
        EXPECT_THROWS_AS(interpolate(*make("nn"), input, *out), exception::SeriousBug);
    }


    SECTION("multiple fields") {
        data::MIRField two(in, false, MISSING);
        auto a = values;
        auto b = values;
        std::for_each(b.begin(), b.end(), [](double& v) { v *= 2.; });
        two.update(a, 0);
        two.update(b, 1);

        auto out    = representation("{grid: [10, 10], area: [5, 5, -5, 15]}");
        auto result = interpolate(
            *make("{type: k-nearest, nearest-method: nclosest, nclosest: 4, distance-weighting: 'no'}"), two, *out);

        EXPECT_EQUAL(result.dimensions(), 2);
        EXPECT(result.values(0) == MIRValuesVector({2., 3., 5., 6.}));
        EXPECT(result.values(1) == MIRValuesVector({4., 6., 10., 12.}));
    }
}


CASE("MIR-425: nearest neighbour supersampling, of values that are not missing") {
    /*
     * interpolate from 4 (i) to 9 (o) points (supersampling);
     * the nearest neighbour method should introduce 5 missing values
     *
     *   i i    ooo
     *       >> ooo
     *   i i    ooo
     */
    auto in  = representation("{grid: [2, 2], area: [2, 0, 0, 2]}");
    auto out = representation("{grid: [1, 1], area: [2, 0, 0, 2]}");
    ASSERT(in->numberOfPoints() == 4);
    ASSERT(out->numberOfPoints() == 9);

    // 9999's as valid (not missing) values
    auto input = field(in, MIRValuesVector(4, MISSING), -1.);
    EXPECT(!input.hasMissing());


    SECTION("default interpolation, should not contain missing values") {
        auto result = interpolate(*make("linear"), input, *out);
        EXPECT(!result.hasMissing());
    }


    SECTION("interpolation=nn, should not contain missing values") {
        auto result = interpolate(*make("nn"), input, *out);
        EXPECT(!result.hasMissing());
        EXPECT(result.values(0) == MIRValuesVector(9, MISSING));
    }


    SECTION("interpolation=nn (custom), should contain missing values") {
        // in [m], 5/9 (o) points do not have neighbours within distance
        auto result = interpolate(*make("{type: nn, nearest-method: distance, distance: 1}"), input, *out);
        EXPECT(result.hasMissing());
        EXPECT_EQUAL(count_missing(result), 5);
    }
}


}  // namespace mir::interpolation::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
