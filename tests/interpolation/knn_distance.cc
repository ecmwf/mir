// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/log/JSON.h"
#include "eckit/testing/Test.h"
#include "eckit/utils/MD5.h"

#include "mir/lsm/LandSeaMasks.h"
#include "mir/lsm/Mask.h"
#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/method/knn/distance/DistanceWeightingWithLSM.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"

#include "interpolation.h"


namespace mir::interpolation::tests {


using method::WeightMatrix;
using method::knn::distance::DistanceWeighting;
using method::knn::distance::DistanceWeightingFactory;
using method::knn::distance::DistanceWeightingWithLSM;
using method::knn::distance::DistanceWeightingWithLSMFactory;

using D         = std::unique_ptr<const DistanceWeighting>;
using Neighbors = std::vector<search::PointSearch::PointValueType>;
using Triplets  = std::vector<WeightMatrix::Triplet>;


struct Weight {
    size_t col;
    double value;
};


bool same_weights(const Triplets& triplets, size_t row, const std::vector<Weight>& expected) {
    if (triplets.size() != expected.size()) {
        Log::info() << "triplets: " << triplets.size() << " != " << expected.size() << std::endl;
        return false;
    }

    for (size_t i = 0; i < expected.size(); ++i) {
        const auto& t = triplets[i];
        if (t.row() != row || t.col() != expected[i].col || !same(t.value(), expected[i].value, 1e-12)) {
            Log::info() << "triplet " << i << ": (" << t.row() << ", " << t.col() << ", " << t.value() << ") != ("
                        << row << ", " << expected[i].col << ", " << expected[i].value << ")" << std::endl;
            return false;
        }
    }

    return true;
}


std::string json(const DistanceWeighting& d) {
    std::ostringstream str;
    eckit::JSON j(str);
    j.startObject();
    j << d;
    j.endObject();
    return str.str();
}


CASE("distance weighting methods") {
    const size_t ip = 7;

    // three neighbours, at squared distances 1, 4 and 9 (from the origin), and 0, 5 and 10 (from point2)
    const Point3 origin{0., 0., 0.};
    const Point3 point2{1., 0., 0.};
    const Neighbors neighbours{
        {{1., 0., 0.}, 1},
        {{0., 2., 0.}, 2},
        {{0., 0., 3.}, 3},
    };

    param::SimpleParametrisation param;

    auto weights = [&](const std::string& name, const Point3& p) {
        D d(DistanceWeightingFactory::build(name, param));
        Triplets triplets;
        (*d)(ip, p, neighbours, triplets);
        return triplets;
    };

    auto normalise = [](std::vector<Weight> w) {
        double sum = 0.;
        for (const auto& x : w) {
            sum += x.value;
        }
        for (auto& x : w) {
            x.value /= sum;
        }
        return w;
    };


    SECTION("inverse-distance-weighting (power 1)") {
        EXPECT(same_weights(weights("inverse-distance-weighting", origin), ip,
                            normalise({{1, 1.}, {2, 1. / 2.}, {3, 1. / 3.}})));

        // an exact match is the only weight
        EXPECT(same_weights(weights("inverse-distance-weighting", point2), ip, {{1, 1.}}));
    }


    SECTION("reciprocal, shepard (power 2)") {
        const auto expected = normalise({{1, 1.}, {2, 1. / 4.}, {3, 1. / 9.}});
        EXPECT(same_weights(weights("reciprocal", origin), ip, expected));
        EXPECT(same_weights(weights("shepard", origin), ip, expected));

        param.set("distance-weighting-shepard-power", 3.);
        EXPECT(same_weights(weights("shepard", origin), ip, normalise({{1, 1.}, {2, 1. / 8.}, {3, 1. / 27.}})));
    }


    SECTION("inverse-distance-weighting-squared") {
        EXPECT(same_weights(weights("inverse-distance-weighting-squared", origin), ip,
                            normalise({{1, 1. / 2.}, {2, 1. / 5.}, {3, 1. / 10.}})));
        EXPECT(same_weights(weights("inverse-distance-weighting-squared", point2), ip,
                            normalise({{1, 1.}, {2, 1. / 6.}, {3, 1. / 11.}})));
    }


    SECTION("nearest-neighbour") {
        EXPECT(same_weights(weights("nearest-neighbour", origin), ip, {{1, 1.}}));
    }


    SECTION("no (no-distance-weighting)") {
        for (const auto& name : std::vector<std::string>{"no", "no-distance-weighting"}) {
            EXPECT(same_weights(weights(name, origin), ip, {{1, 1. / 3.}, {2, 1. / 3.}, {3, 1. / 3.}}));
        }
    }


    SECTION("gaussian") {
        param.set("distance-weighting-gaussian-stddev", 1.);
        EXPECT(same_weights(weights("gaussian", origin), ip,
                            normalise({{1, std::exp(-0.5)}, {2, std::exp(-2.)}, {3, std::exp(-4.5)}})));

        // the standard deviation defaults to the distance
        param::SimpleParametrisation other;
        other.set("distance", 1.);
        D a(DistanceWeightingFactory::build("gaussian", param));
        D b(DistanceWeightingFactory::build("gaussian", other));
        EXPECT(a->sameAs(*b));
    }


    SECTION("cressman") {
        param.set("distance", 3.);

        auto c = [](double d2, double power) { return std::pow((9. - d2) / (9. + d2), power); };
        EXPECT(same_weights(weights("cressman", origin), ip, normalise({{1, c(1., 1.)}, {2, c(4., 1.)}, {3, 0.}})));

        param.set("cressman-model-extension-power", 2.);
        EXPECT(same_weights(weights("cressman", origin), ip, normalise({{1, c(1., 2.)}, {2, c(4., 2.)}, {3, 0.}})));

        param.set("cressman-model-extension-power", 1.5);
        EXPECT(same_weights(weights("cressman", origin), ip, normalise({{1, c(1., 1.5)}, {2, c(4., 1.5)}, {3, 0.}})));

        // no neighbours within distance: the nearest neighbour
        param.set("distance", 0.5);
        EXPECT(same_weights(weights("cressman", origin), ip, {{1, 1.}}));

        param.set("cressman-model-extension-power", 0.5);
        EXPECT_THROWS(weights("cressman", origin));
    }


    SECTION("climate-filter") {
        // half distance 3, delta 2: weight 1 up to 1, cosine down to 0 at 5
        param.set("distance", 6.);
        param.set("climate-filter-delta", 2.);

        auto h = [](double r) { return 0.5 + 0.5 * std::cos(M_PI_2 * (r - 1.) / 2.); };
        EXPECT(same_weights(weights("climate-filter", origin), ip, normalise({{1, h(1.)}, {2, h(2.)}, {3, h(3.)}})));

        param.set("distance", 1.);
        EXPECT_THROWS_AS(weights("climate-filter", origin), exception::UserError);
    }


    SECTION("pseudo-laplace") {
        // weights summing zero: nearest neighbour
        EXPECT(same_weights(weights("pseudo-laplace", origin), ip, {{1, 1.}}));

        // singular moments (neighbours on a plane through the point): nearest neighbour
        D d(DistanceWeightingFactory::build("pseudo-laplace", param));
        Triplets triplets;
        (*d)(ip, origin, {neighbours[0], neighbours[1]}, triplets);
        EXPECT(same_weights(triplets, ip, {{1, 1.}}));

        // weights reproduce linear functions: sum(w) = 1 and sum(w * x) = p
        const Neighbors cube{
            {{0., 0., 0.}, 0}, {{1., 0., 0.}, 1}, {{0., 1., 0.}, 2}, {{0., 0., 1.}, 3}, {{1., 1., 1.}, 4},
        };
        const Point3 p{0.2, 0.3, 0.1};

        triplets.clear();
        (*d)(ip, p, cube, triplets);
        EXPECT_EQUAL(triplets.size(), cube.size());

        double sum = 0.;
        std::vector<double> x(3, 0.);
        for (size_t i = 0; i < triplets.size(); ++i) {
            const auto w = triplets[i].value();
            sum += w;
            for (size_t c = 0; c < 3; ++c) {
                x[c] += w * cube[i].point()[c];
            }
        }

        EXPECT(same(sum, 1.));
        EXPECT(same(x[0], p[0]) && same(x[1], p[1]) && same(x[2], p[2]));
    }


    SECTION("matrix validation") {
        for (const auto& name : std::vector<std::string>{"inverse-distance-weighting", "nearest-neighbour", "no"}) {
            auto check = D(DistanceWeightingFactory::build(name, param))->validateMatrixWeights();
            EXPECT(check.duplicates && check.bounds && check.sum);
        }

        // unbounded weights
        auto check = D(DistanceWeightingFactory::build("pseudo-laplace", param))->validateMatrixWeights();
        EXPECT(check.duplicates && !check.bounds && !check.sum);
    }


    SECTION("comparison, hash, print and json") {
        param.set("distance", 2.);
        param.set("climate-filter-delta", 1.);

        const std::vector<std::string> names{"climate-filter",
                                             "cressman",
                                             "gaussian",
                                             "inverse-distance-weighting",
                                             "inverse-distance-weighting-squared",
                                             "nearest-neighbour",
                                             "no",
                                             "pseudo-laplace"};

        for (const auto& name : names) {
            D d(DistanceWeightingFactory::build(name, param));

            for (const auto& other : names) {
                D o(DistanceWeightingFactory::build(other, param));
                EXPECT(d->sameAs(*o) == (name == other));

                eckit::MD5 h1;
                eckit::MD5 h2;
                d->hash(h1);
                o->hash(h2);
                EXPECT((h1.digest() == h2.digest()) == (name == other));
            }

            std::ostringstream str;
            str << *d;
            EXPECT(!str.str().empty());

            EXPECT(json(*d).find(R"("distance-weighting":")" + name + "\"") != std::string::npos);
        }
    }


    SECTION("different options") {
        param::SimpleParametrisation a;
        a.set("distance", 2.);
        a.set("climate-filter-delta", 1.);
        a.set("distance-weighting-gaussian-stddev", 1.);

        param::SimpleParametrisation b;
        b.set("distance", 3.);
        b.set("climate-filter-delta", 1.);
        b.set("cressman-model-extension-power", 2.);
        b.set("distance-weighting-gaussian-stddev", 2.);

        for (const auto& name : std::vector<std::string>{"climate-filter", "cressman", "gaussian"}) {
            D x(DistanceWeightingFactory::build(name, a));
            D y(DistanceWeightingFactory::build(name, b));
            EXPECT(!x->sameAs(*y));
        }

        D x(DistanceWeightingFactory::build("inverse-distance-weighting", a));
        D y(DistanceWeightingFactory::build("reciprocal", a));
        EXPECT(!x->sameAs(*y));

        std::ostringstream str;
        str << *D(DistanceWeightingFactory::build("cressman", b));
        EXPECT_EQUAL(str.str(), "Cressman[radius=3,power=2]");
    }


    SECTION("unknown") {
        EXPECT_THROWS_AS(weights("unknown", origin), exception::SeriousBug);
    }
}


CASE("distance weighting methods, as method options") {
    struct test_t {
        std::string options;
        std::string expected;
    };

    for (const auto& test : std::vector<test_t>{
             {"distance-weighting: climate-filter, distance: 2, climate-filter-delta: 1",
              R"("distance-weighting":"climate-filter","climate-filter-delta":1)"},
             {"distance-weighting: cressman, distance: 2, cressman-model-extension-power: 2",
              R"("distance-weighting":"cressman","distance":2,"cressman-model-extension-power":2)"},
             {"distance-weighting: gaussian, distance-weighting-gaussian-stddev: 2",
              R"("distance-weighting":"gaussian","distance-weighting-gaussian-stddev":2)"},
             {"distance-weighting: inverse-distance-weighting",
              R"("distance-weighting":"inverse-distance-weighting","inverse-distance-weighting-power":1)"},
             {"distance-weighting: inverse-distance-weighting-squared",
              R"("distance-weighting":"inverse-distance-weighting-squared")"},
             {"distance-weighting: nearest-neighbour", R"("distance-weighting":"nearest-neighbour")"},
             {"distance-weighting: 'no'", R"("distance-weighting":"no")"},
             {"distance-weighting: pseudo-laplace", R"("distance-weighting":"pseudo-laplace")"},
         }) {
        auto m = make("{type: k-nearest, " + test.options + "}");
        EXPECT(m->json_str(false).find(test.expected) != std::string::npos);
        EXPECT(*make(m->json_str()) == *m);

        std::ostringstream str;
        str << *m;
        EXPECT(str.str().find("distanceWeighting=") != std::string::npos);
    }


    SECTION("interpolating with unbounded weights (pseudo-laplace)") {
        auto in  = representation("{grid: [10, 10], area: [10, 0, -10, 20]}");
        auto out = representation("{grid: [2, 2], area: [8, 2, -8, 18]}");

        // a constant is reproduced
        auto result =
            interpolate(*make("{type: k-nearest, nearest-method: nclosest, distance-weighting: pseudo-laplace}"),
                        field(in, MIRValuesVector(in->numberOfPoints(), 3.)), *out);

        for (const auto& v : result.values(0)) {
            EXPECT(same(v, 3., 1e-6));
        }
    }
}


struct TestMask final : lsm::Mask {
    explicit TestMask(const std::vector<bool>& mask) : mask_(mask) {}

    bool active() const override { return true; }
    bool cacheable() const override { return false; }
    const std::vector<bool>& mask() const override { return mask_; }
    std::string cacheName() const override { return "test"; }
    void print(std::ostream& out) const override { out << "TestMask[]"; }

    const std::vector<bool> mask_;
};


CASE("distance weighting methods, with land-sea masks") {
    // input points 0 to 3 (land, land, sea, sea), output points 0 and 1 (land, sea)
    const TestMask imask({true, true, false, false});
    const TestMask omask({true, false});
    const lsm::LandSeaMasks masks(imask, omask);

    EXPECT(masks.active());
    EXPECT(!masks.cacheable());
    EXPECT_EQUAL(masks.cacheName(), "test+test");

    // neighbours sorted by distance (from the origin), the farthest with the lowest index
    const Point3 origin{0., 0., 0.};
    const Neighbors neighbours{
        {{1., 0., 0.}, 3},
        {{0., 2., 0.}, 1},
        {{0., 0., 3.}, 0},
        {{0., 0., 4.}, 2},
    };

    param::SimpleParametrisation param;

    auto pick = [&](const std::string& name, size_t ip, const Neighbors& n) {
        param.set("distance-weighting-with-lsm", name);
        D d(DistanceWeightingWithLSMFactory::build(name, param, masks));

        Triplets triplets;
        (*d)(ip, origin, n, triplets);
        EXPECT_EQUAL(triplets.size(), 1);
        EXPECT(same(triplets.front().value(), 1.));
        return triplets.front().col();
    };


    SECTION("nearest-lsm: the closest of the same type") {
        EXPECT_EQUAL(pick("nearest-lsm", 0, neighbours), 1);
        EXPECT_EQUAL(pick("nearest-lsm", 1, neighbours), 3);

        // none of the same type: the closest
        EXPECT_EQUAL(pick("nearest-lsm", 0, {neighbours[0], neighbours[3]}), 3);
    }


    SECTION("nearest-lsm-with-lowest-index: the closest of the same type, the lowest index if equidistant") {
        EXPECT_EQUAL(pick("nearest-lsm-with-lowest-index", 0, neighbours), 1);
        EXPECT_EQUAL(pick("nearest-lsm-with-lowest-index", 1, neighbours), 3);
        EXPECT_EQUAL(pick("nearest-lsm-with-lowest-index", 0, {neighbours[0], neighbours[3]}), 3);

        const Neighbors equidistant{{{0., 2., 0.}, 1}, {{2., 0., 0.}, 0}, {{1., 0., 0.}, 2}};
        EXPECT_EQUAL(pick("nearest-lsm-with-lowest-index", 0, equidistant), 0);
    }


    SECTION("comparison, hash, print and json") {
        param.set("distance-weighting-with-lsm", "nearest-lsm");
        D a(DistanceWeightingWithLSMFactory::build("nearest-lsm", param, masks));
        D b(DistanceWeightingWithLSMFactory::build("nearest-lsm-with-lowest-index", param, masks));

        EXPECT(a->sameAs(*a));
        EXPECT(!a->sameAs(*b));
        EXPECT(!b->sameAs(*a));
        EXPECT(b->sameAs(*b));

        eckit::MD5 h1;
        eckit::MD5 h2;
        a->hash(h1);
        b->hash(h2);
        EXPECT(h1.digest() != h2.digest());

        std::ostringstream sa;
        std::ostringstream sb;
        sa << *a;
        sb << *b;
        EXPECT_EQUAL(sa.str(), "NearestLSM[]");
        EXPECT_EQUAL(sb.str(), "NearestLSMWithLowestIndex[]");

        EXPECT(json(*a).find(R"("nearest-lsm")") != std::string::npos);
        EXPECT(json(*b).find(R"("nearest-lsm-with-lowest-index")") != std::string::npos);
    }


    SECTION("the (land-sea mask independent) weighting") {
        DistanceWeightingWithLSM a(param);  // default: nearest-lsm-with-lowest-index

        param.set("distance-weighting-with-lsm", "nearest-lsm");
        DistanceWeightingWithLSM b(param);

        const DistanceWeighting& x = a;
        const DistanceWeighting& y = b;
        EXPECT(x.sameAs(x));
        EXPECT(!x.sameAs(y));

        std::ostringstream str;
        str << x;
        EXPECT_EQUAL(str.str(), "DistanceWeightingWithLSM[method=nearest-lsm-with-lowest-index]");

        eckit::MD5 h;
        x.hash(h);

        EXPECT(json(x).find(R"("nearest-lsm-with-lowest-index")") != std::string::npos);

        // the (land-sea mask dependent) weighting it builds
        D d(a.distanceWeighting(param, masks));
        EXPECT(d->sameAs(*D(DistanceWeightingWithLSMFactory::build("nearest-lsm-with-lowest-index", param, masks))));

        // not to be used directly
        Triplets triplets;
        EXPECT_THROWS_AS(x(0, origin, neighbours, triplets), exception::SeriousBug);
    }


    SECTION("unknown") {
        param.set("distance-weighting-with-lsm", "unknown");
        EXPECT_THROWS_AS(DistanceWeightingWithLSM{param}, exception::SeriousBug);
        EXPECT_THROWS_AS(D(DistanceWeightingWithLSMFactory::build("unknown", param, masks)), exception::SeriousBug);

        std::ostringstream list;
        DistanceWeightingWithLSMFactory::list(list);
        EXPECT_EQUAL(list.str(), "nearest-lsm, nearest-lsm-with-lowest-index");
    }
}


}  // namespace mir::interpolation::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
