// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>

#include "action.h"


namespace mir::tests::actions {


const std::string ROTATED{"{grid: [1, 1], area: [10, -10, -10, 10], rotation: [-40, 22]}"};


MIRValuesVector values(const std::string& grid, const F& f) {
    return field(grid, f).values(0);
}


data::MIRField uv(const std::string& grid, const F& u, const F& v) {
    auto f  = field(grid, u, 2);
    auto vv = values(grid, v);
    f.update(vv, 1, true);
    return f;
}


CASE("filter.adjust-winds-directions") {
    SECTION("rotated grid: directions change, magnitude is kept") {
        auto in  = uv(ROTATED, [](double, double) { return 3.; }, [](double, double) { return 4.; });
        auto out = perform("{type: filter.adjust-winds-directions, rotation: [-40, 22]}", in);

        EXPECT_EQUAL(out.dimensions(), 2);
        EXPECT(!out.hasMissing());

        size_t changed = 0;

        for (size_t i = 0; i < out.values(0).size(); ++i) {
            const auto x = out.values(0)[i];
            const auto y = out.values(1)[i];

            EXPECT(same(std::hypot(x, y), 5.));

            changed += same(x, 3.) && same(y, 4.) ? 0 : 1;
        }

        EXPECT(changed > 0);

        for (size_t which = 0; which < 2; ++which) {
            EXPECT_EQUAL(out.metadata(which).at("uvRelativeToGrid"), 1);
        }
    }

    SECTION("pairs of components") {
        auto in = uv(ROTATED, [](double lat, double) { return lat; }, [](double, double lon) { return lon; });
        auto u2 = values(ROTATED, [](double, double) { return 0.; });
        auto v2 = values(ROTATED, [](double, double) { return 1.; });
        in.update(u2, 2);
        in.update(v2, 3);

        auto out = perform("{type: filter.adjust-winds-directions, rotation: [-40, 22]}", in);
        auto ref = perform("{type: filter.adjust-winds-directions, rotation: [-40, 22]}",
                           uv(ROTATED, [](double lat, double) { return lat; }, [](double, double lon) { return lon; }));

        EXPECT_EQUAL(out.dimensions(), 4);
        EXPECT(out.values(0) == ref.values(0));
        EXPECT(out.values(1) == ref.values(1));

        for (size_t i = 0; i < out.values(2).size(); ++i) {
            EXPECT(same(std::hypot(out.values(2)[i], out.values(3)[i]), 1.));
        }

        EXPECT_EQUAL(out.metadata(3).at("uvRelativeToGrid"), 1);
    }

    SECTION("no rotation: unchanged") {
        auto in =
            uv("{grid: [10, 10]}", [](double lat, double) { return lat; }, [](double, double lon) { return lon; });
        auto out = perform("{type: filter.adjust-winds-directions, rotation: [-90, 0]}", in);

        // the angle sine is sqrt(1 - cos^2), amplifying rounding errors
        for (size_t which = 0; which < 2; ++which) {
            for (size_t i = 0; i < out.values(which).size(); ++i) {
                EXPECT(same(out.values(which)[i], in.values(which)[i], 1e-4));
            }
        }
    }
}


CASE("filter.adjust-winds-directions, with missing values") {
    // a missing component makes both components missing
    auto in = uv(
        ROTATED, [](double lat, double) { return lat > 5 ? 9999. : 1.; },
        [](double lat, double) { return lat < -5 ? 9999. : 1.; });
    EXPECT(in.hasMissing());

    auto out = perform("{type: filter.adjust-winds-directions, rotation: [-40, 22]}", in);

    for (size_t i = 0; i < in.values(0).size(); ++i) {
        const bool missing = in.values(0)[i] == 9999. || in.values(1)[i] == 9999.;

        EXPECT((out.values(0)[i] == 9999.) == missing);
        EXPECT((out.values(1)[i] == 9999.) == missing);
    }
}


CASE("filter.adjust-winds-directions, properties") {
    auto a = make("{type: filter.adjust-winds-directions, rotation: [-40, 22]}");

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.adjust-winds-directions","rotation":[-40,22]})");
    EXPECT(*a == *make("{type: filter.adjust-winds-directions, rotation: -40/22}"));
    EXPECT(*a != *make("{type: filter.adjust-winds-directions, rotation: [-40, 23]}"));
    EXPECT(*a != *make("filter.adjust-winds-scale-cos-latitude"));
}


CASE("filter.adjust-winds-directions, errors") {
    EXPECT_THROWS(make("filter.adjust-winds-directions"));  // rotation is required
    EXPECT_THROWS(make("{type: filter.adjust-winds-directions, rotation: [-40, 22, 0]}"));

    // pairs of components are required
    EXPECT_THROWS(perform("{type: filter.adjust-winds-directions, rotation: [-40, 22]}", field(ROTATED)));
}


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
