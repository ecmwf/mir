// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/utils/MD5.h"

#include "mir/lsm/LSMSelection.h"
#include "mir/lsm/LandSeaMasks.h"
#include "mir/lsm/Mask.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"

#include "interpolation.h"


namespace mir::interpolation::tests {


using lsm::LandSeaMasks;
using lsm::LSMSelection;
using lsm::Mask;


// HEALPix H2 (ring), values 1 to 48 (in tests/data): with threshold 24.5, the last 24 points are "land"
const std::string LSM_FILE            = "gridType=healpix,Nside=2,orderingConvention=ring.grib2";
const std::string LSM_FILE_NOT_GLOBAL = "regular_ll.5-5.area.grib1";


void set_file_lsm(param::SimpleParametrisation& param, const std::string& path = LSM_FILE) {
    param.set("lsm", true);
    param.set("lsm-selection", "file");
    param.set("lsm-file", path);
    param.set("lsm-value-threshold", 24.5);
    param.set("lsm-interpolation", "nearest-neighbour");
}


size_t count_true(const std::vector<bool>& mask) {
    return static_cast<size_t>(std::count(mask.begin(), mask.end(), true));
}


CASE("land-sea masks: not active") {
    auto r = representation("{grid: [30, 30]}");


    SECTION("lsm: false (default)") {
        param::SimpleParametrisation param;

        for (const auto& lsm : std::vector<bool>{false, true}) {
            if (lsm) {
                param.set("lsm", false);
            }

            const auto& mask = Mask::lookupInput(param, *r);
            EXPECT(!mask.active());
            EXPECT(!mask.cacheable());
            EXPECT_THROWS(mask.mask());
            EXPECT_THROWS(mask.cacheName());

            eckit::MD5 h;
            EXPECT_THROWS(mask.hash(h));

            std::ostringstream str;
            str << mask;
            EXPECT_EQUAL(str.str(), "NoMask[]");

            auto masks = LandSeaMasks::lookup(param, *r, *r);
            EXPECT(!masks.active());
            EXPECT(!masks.cacheable());
        }
    }


    SECTION("lsm-selection: none") {
        param::SimpleParametrisation param;
        param.set("lsm", true);
        param.set("lsm-selection", "none");

        const auto& in  = Mask::lookupInput(param, *r);
        const auto& out = Mask::lookupOutput(param, *r);
        EXPECT(!in.active());
        EXPECT(!out.active());
        EXPECT(&in == &out);

        const auto& none = LSMSelection::lookup("none");
        EXPECT_EQUAL(none.cacheKey(param, *r, "input"), "none");

        std::ostringstream str;
        str << none;
        EXPECT_EQUAL(str.str(), "NoneLSM[none]");
    }


    SECTION("comparison") {
        param::SimpleParametrisation a;
        param::SimpleParametrisation b;
        b.set("lsm", true);

        EXPECT(LandSeaMasks::sameLandSeaMasks(a, a));
        EXPECT(LandSeaMasks::sameLandSeaMasks(b, b));
        EXPECT(!LandSeaMasks::sameLandSeaMasks(a, b));
        EXPECT(Mask::sameInput(b, b) && Mask::sameOutput(b, b));
        EXPECT(!Mask::sameInput(a, b) && !Mask::sameOutput(b, a));
    }
}


CASE("land-sea masks: selection") {
    auto r = representation("{grid: [30, 30]}");


    SECTION("lsm-selection is required") {
        param::SimpleParametrisation param;
        param.set("lsm", true);
        EXPECT_THROWS_AS(Mask::lookupInput(param, *r), exception::SeriousBug);
    }


    SECTION("lsm-selection: unknown") {
        param::SimpleParametrisation param;
        param.set("lsm", true);
        param.set("lsm-selection", "unknown");
        EXPECT_THROWS_AS(Mask::lookupInput(param, *r), exception::SeriousBug);

        std::ostringstream list;
        LSMSelection::list(list);
        EXPECT(list.str().find("file") != std::string::npos);
        EXPECT(list.str().find("named") != std::string::npos);
        EXPECT(list.str().find("none") != std::string::npos);
    }


    SECTION("lsm-selection: named (cache key, which does not require the mask)") {
        param::SimpleParametrisation param;
        param.set("lsm-named", "1km");
        param.set("lsm-interpolation", "nearest-neighbour");

        const auto& named = LSMSelection::lookup("named");

        auto key = named.cacheKey(param, *r, "input");
        EXPECT(key.rfind("named.1km.", 0) == 0);

        // by which (input/output), and case-insensitive
        param.set("lsm-named-output", "10MIN");
        EXPECT(named.cacheKey(param, *r, "output").rfind("named.10min.", 0) == 0);

        // a different interpolation, a different key
        param.set("lsm-interpolation-input", "linear");
        EXPECT(named.cacheKey(param, *r, "input") != key);

        std::ostringstream str;
        str << named;
        EXPECT_EQUAL(str.str(), "NamedLSM[name=named]");
    }


    SECTION("lsm-selection: named, unknown") {
        param::SimpleParametrisation param;
        param.set("lsm", true);
        param.set("lsm-selection", "named");
        param.set("lsm-named", "unknown");
        param.set("lsm-interpolation", "nearest-neighbour");

        EXPECT_THROWS(Mask::lookupInput(param, *r));
        EXPECT_THROWS(LSMSelection::lookup("named").cacheKey(param, *r, "input"));
    }


    SECTION("lsm-interpolation is required") {
        param::SimpleParametrisation param;
        param.set("lsm-file", LSM_FILE);
        EXPECT_THROWS_AS(LSMSelection::lookup("file").cacheKey(param, *r, "input"), exception::SeriousBug);
    }


    SECTION("selection registration") {
        struct TestSelection final : LSMSelection {
            TestSelection() : LSMSelection("test") {}
            Mask* create(const param::MIRParametrisation& param, const repres::Representation& r,
                         const std::string& which) const override {
                return LSMSelection::lookup("none").create(param, r, which);
            }
            std::string cacheKey(const param::MIRParametrisation&, const repres::Representation&,
                                 const std::string&) const override {
                return "test";
            }
            void print(std::ostream& out) const override { out << "TestSelection[]"; }
        };

        {
            TestSelection test;
            EXPECT(&LSMSelection::lookup("test") == &test);
        }

        EXPECT_THROWS_AS(LSMSelection::lookup("test"), exception::SeriousBug);
    }
}


CASE("land-sea masks: from a file") {
    auto H2 = representation("{grid: H2}");
    auto ll = representation("{grid: [30, 30]}");


    SECTION("the same representation (mask from the values)") {
        param::SimpleParametrisation param_user;
        set_file_lsm(param_user);
        const param::CombinedParametrisation param(param_user);
        const auto& mask = Mask::lookupInput(param, *H2);

        EXPECT(mask.active());
        EXPECT(!mask.cacheable());
        EXPECT_EQUAL(mask.cacheName(), LSM_FILE);
        EXPECT_EQUAL(mask.mask().size(), 48);
        EXPECT_EQUAL(count_true(mask.mask()), 24);
        EXPECT(std::all_of(mask.mask().begin() + 24, mask.mask().end(), [](bool b) { return b; }));

        std::ostringstream str;
        str << mask;
        EXPECT_EQUAL(str.str(), "GribFileMask[path=" + LSM_FILE + "]");

        eckit::MD5 h;
        mask.hash(h);

        // cached
        EXPECT(&Mask::lookupInput(param, *H2) == &mask);
    }


    SECTION("another representation (mask interpolated)") {
        param::SimpleParametrisation param_user;
        set_file_lsm(param_user);
        const param::CombinedParametrisation param(param_user);
        const auto& mask = Mask::lookupOutput(param, *ll);

        EXPECT_EQUAL(mask.mask().size(), ll->numberOfPoints());
        EXPECT(0 < count_true(mask.mask()) && count_true(mask.mask()) < ll->numberOfPoints());

        // the north pole is "sea", the south pole "land"
        EXPECT(!mask.mask().front());
        EXPECT(mask.mask().back());
    }


    SECTION("options by which (input/output)") {
        param::SimpleParametrisation param_user;
        set_file_lsm(param_user);
        const param::CombinedParametrisation param(param_user);
        param_user.set("lsm-value-threshold-output", 0.);

        // (a representation not used before: the masks cache key does not consider the threshold)
        auto other = representation("{grid: [45, 45]}");

        const auto& in  = Mask::lookupInput(param, *H2);
        const auto& out = Mask::lookupOutput(param, *other);
        EXPECT_EQUAL(count_true(in.mask()), 24);
        EXPECT_EQUAL(count_true(out.mask()), other->numberOfPoints());

        param_user.set("lsm-file-input", LSM_FILE_NOT_GLOBAL);
        param_user.set("lsm-file", "unused");
        EXPECT_THROWS_AS(Mask::lookupInput(param, *ll), exception::UserError);
    }


    SECTION("the file is required, and global (to be interpolated)") {
        param::SimpleParametrisation param_user;
        set_file_lsm(param_user, "");
        const param::CombinedParametrisation param(param_user);
        EXPECT_THROWS_AS(Mask::lookupInput(param, *ll), exception::UserError);

        param::SimpleParametrisation other;

        set_file_lsm(other, LSM_FILE_NOT_GLOBAL);
        EXPECT_THROWS_AS(Mask::lookupInput(other, *ll), exception::UserError);
    }


    SECTION("input and output masks") {
        param::SimpleParametrisation param_user;
        set_file_lsm(param_user);
        const param::CombinedParametrisation param(param_user);
        auto masks = LandSeaMasks::lookup(param, *H2, *ll);

        EXPECT(masks.active());
        EXPECT(!masks.cacheable());
        EXPECT_EQUAL(masks.cacheName(), LSM_FILE + "+" + LSM_FILE);
        EXPECT_EQUAL(masks.inputMask().size(), H2->numberOfPoints());
        EXPECT_EQUAL(masks.outputMask().size(), ll->numberOfPoints());

        std::ostringstream str;
        str << masks;
        EXPECT(str.str().rfind("LandSeaMasks[in=GribFileMask[", 0) == 0);

        eckit::MD5 h;
        masks.hash(h);
    }
}


CASE("land-sea masks, interpolating") {
    auto H2 = representation("{grid: H2}");
    auto ll = representation("{grid: [20, 20]}");

    // output "land" is less than input "land", so the nearest neighbours are not always of the same type
    const std::string lsm = ", lsm-selection: file, lsm-file: '" + LSM_FILE +
                            "', lsm-value-threshold: 24.5, lsm-value-threshold-output: 36.5}";

    param::SimpleParametrisation user;
    set_file_lsm(user);
    user.set("lsm-value-threshold-output", 36.5);

    const param::CombinedParametrisation param(user);
    auto masks = LandSeaMasks::lookup(param, *H2, *ll);

    const auto& imask = masks.inputMask();
    const auto& omask = masks.outputMask();


    SECTION("matrix-based methods: weights of a different type are adjusted") {
        const std::string knn = "{type: k-nearest, nearest-method: nclosest, nclosest: 4, distance-weighting: 'no'";

        auto m = make(knn + ", lsm: true" + lsm);
        EXPECT(m->json_str().find(R"("lsm":true)") != std::string::npos);

        auto W  = matrix(*m, *H2, *ll);
        auto W0 = matrix(*make(knn + "}"), *H2, *ll);
        EXPECT_EQUAL(W.nonZeros(), W0.nonZeros());

        size_t adjusted = 0;
        for (size_t r = 0; r < W0.rows(); ++r) {
            double sum = 0.;
            for (auto it = W0.begin(r); it != W0.end(r); ++it) {
                sum += *it * (imask[it.col()] == omask[r] ? 1. : 0.2);
            }

            for (auto it = W0.begin(r); it != W0.end(r); ++it) {
                auto expected = *it * (imask[it.col()] == omask[r] ? 1. : 0.2) / sum;
                EXPECT(same(weight(W, r, it.col()), expected));
                adjusted += imask[it.col()] == omask[r] ? 0 : 1;
            }
        }

        EXPECT(adjusted > 0);
    }


    SECTION("nearest-lsm: the nearest of the same type") {
        auto nn = matrix(*make("nn"), *H2, *ll);

        auto matches = [&](const method::WeightMatrix& W) {
            size_t n = 0;
            for (size_t r = 0; r < W.rows(); ++r) {
                EXPECT(W.begin(r) != W.end(r));
                for (auto it = W.begin(r); it != W.end(r); ++it) {
                    EXPECT(*it == 1.);
                    n += imask[it.col()] == omask[r] ? 1 : 0;
                }
            }
            return n;
        };

        for (const auto& weighting : std::vector<std::string>{"nearest-lsm", "nearest-lsm-with-lowest-index"}) {
            auto m = make("{type: nearest-lsm, distance-weighting-with-lsm: " + weighting + lsm);
            EXPECT(m->sameAs(*make("{type: nearest-lsm, distance-weighting-with-lsm: " + weighting + lsm)));

            auto W = matrix(*m, *H2, *ll);
            EXPECT_EQUAL(W.nonZeros(), ll->numberOfPoints());
            EXPECT(matches(W) > matches(nn));

            auto input  = field(H2, [](double lat, double) { return lat; });
            auto result = interpolate(*m, input, *ll);
            for (const auto& v : result.values(0)) {
                EXPECT(std::find(input.values(0).begin(), input.values(0).end(), v) != input.values(0).end());
            }
        }

        std::ostringstream str;
        str << *make("{type: nearest-lsm" + lsm);
        EXPECT(str.str().find("DistanceWeightingWithLSM[method=nearest-lsm-with-lowest-index]") != std::string::npos);
    }
}


}  // namespace mir::interpolation::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
