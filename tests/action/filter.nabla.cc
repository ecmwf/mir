// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <limits>
#include <vector>

#include "mir/util/Angles.h"

#include "action.h"
#include "winds.h"


namespace mir::action::tests {


CASE("filter.scalar-gradient") {
    SECTION("meridional") {
        auto out = perform("filter.scalar-gradient",
                           field(GRID, [](double lat, double) { return R * util::degree_to_radian(lat); }));

        EXPECT_EQUAL(out.dimensions(), 2);
        EXPECT(!out.hasMissing());

        expect_near(out, 0, [](double, double) { return 0.; }, 1e-6);
        expect_near(out, 1, [](double, double) { return 1.; }, 0.05);
    }

    SECTION("constant") {
        auto out = perform("filter.scalar-gradient", field(GRID, [](double, double) { return 1.; }));
        expect_near(out, 0, [](double, double) { return 0.; }, 1e-12);
        expect_near(out, 1, [](double, double) { return 0.; }, 1e-12);
    }
}


CASE("filter.scalar-laplacian") {
    // spherical harmonic of degree 1: eigenvalue -n (n + 1) / R^2
    auto out = perform("filter.scalar-laplacian", field(GRID, [](double lat, double) { return sind(lat); }));

    EXPECT_EQUAL(out.dimensions(), 1);

    expect_near(out, 0, [](double lat, double) { return -2. * sind(lat) / (R * R); }, 0.05 * 2. / (R * R));
}


CASE("filter.uv-gradient") {
    // solid body rotation
    auto out = perform("filter.uv-gradient",
                       uv(GRID, [](double lat, double) { return cosd(lat); }, [](double, double) { return 0.; }));

    EXPECT_EQUAL(out.dimensions(), 4);

    expect_near(out, 0, [](double, double) { return 0.; }, 1e-12);
    expect_near(out, 1, [](double lat, double) { return -sind(lat) / R; }, 0.05 / R);
    expect_near(out, 2, [](double, double) { return 0.; }, 1e-12);
    expect_near(out, 3, [](double, double) { return 0.; }, 1e-12);
}


CASE("filter.uv-divergence") {
    SECTION("solid body rotation: non-divergent") {
        auto out = perform("filter.uv-divergence",
                           uv(GRID, [](double lat, double) { return cosd(lat); }, [](double, double) { return 0.; }));
        EXPECT_EQUAL(out.dimensions(), 1);

        expect_near(out, 0, [](double, double) { return 0.; }, 1e-12);
    }

    SECTION("meridional flow") {
        auto out = perform("filter.uv-divergence",
                           uv(GRID, [](double, double) { return 0.; }, [](double lat, double) { return cosd(lat); }));
        EXPECT_EQUAL(out.dimensions(), 1);

        expect_near(out, 0, [](double lat, double) { return -2. * sind(lat) / R; }, 0.05 * 2. / R);
    }
}


CASE("filter.uv-vorticity") {
    SECTION("solid body rotation") {
        auto out = perform("filter.uv-vorticity",
                           uv(GRID, [](double lat, double) { return cosd(lat); }, [](double, double) { return 0.; }));
        EXPECT_EQUAL(out.dimensions(), 1);

        expect_near(out, 0, [](double lat, double) { return 2. * sind(lat) / R; }, 0.05 * 2. / R);
    }

    SECTION("meridional flow: irrotational") {
        auto out = perform("filter.uv-vorticity",
                           uv(GRID, [](double, double) { return 0.; }, [](double lat, double) { return cosd(lat); }));
        expect_near(out, 0, [](double, double) { return 0.; }, 1e-12);
    }
}


CASE("filter.*, nabla-poles-missing-values") {
    struct test_t {
        std::string type;
        size_t dimensionsIn;
        size_t dimensionsOut;
    };

    const std::vector<test_t> tests{
        test_t{"filter.scalar-gradient", 1, 2}, test_t{"filter.scalar-laplacian", 1, 1},
        test_t{"filter.uv-gradient", 2, 4},     test_t{"filter.uv-divergence", 2, 1},
        test_t{"filter.uv-vorticity", 2, 1},
    };
    for (const auto& test : tests) {
        SECTION(test.type) {
            // 36 points on each pole
            auto in = field("{grid: [10, 10]}", [](double lat, double) { return cosd(lat); }, test.dimensionsIn);

            auto out = perform("{type: " + test.type + ", nabla-poles-missing-values: true}", in);

            EXPECT_EQUAL(out.dimensions(), test.dimensionsOut);
            EXPECT(out.hasMissing());
            EXPECT(out.missingValue() == std::numeric_limits<double>::min());

            for (size_t which = 0; which < out.dimensions(); ++which) {
                EXPECT_EQUAL(count_missing(out, which), 2 * 36);
            }

            auto ref = perform(test.type, in);

            EXPECT(!ref.hasMissing());
            EXPECT_EQUAL(ref.dimensions(), test.dimensionsOut);
        }
    }
}


CASE("filter.*, properties") {
    const std::vector<std::string> types{"filter.scalar-gradient", "filter.scalar-laplacian", "filter.uv-gradient",
                                         "filter.uv-divergence", "filter.uv-vorticity"};
    for (const std::string& type : types) {
        SECTION(type) {
            auto a = make_checked(type);
            auto b = make_checked("{type: " + type + ", nabla-poles-missing-values: true}");

            EXPECT_EQUAL(a->json_str(), type);
            EXPECT_EQUAL(b->json_str(), R"({"type":")" + type + R"(","nabla-poles-missing-values":true})");

            EXPECT(*a == *make("{type: " + type + ", nabla-poles-missing-values: false}"));
            EXPECT(*a != *b);
        }
    }

    EXPECT(*make("filter.uv-divergence") != *make("filter.uv-vorticity"));
    EXPECT(*make("filter.uv-gradient") != *make("filter.scalar-gradient"));
}


CASE("filter.*, errors") {
    // missing values are not supported
    auto in = field(GRID, [](double lat, double) { return lat > 50 ? 9999. : lat; }, 2, 9999.);

    const std::vector<std::string> types{"filter.scalar-gradient", "filter.uv-divergence"};
    for (const std::string& type : types) {
        EXPECT_THROWS(perform(type, in));
    }

    // number of components
    EXPECT_THROWS(perform("filter.scalar-gradient", field(GRID, nullptr, 2)));
    EXPECT_THROWS(perform("filter.uv-vorticity", field(GRID, nullptr, 1)));
}


CASE("filter.*, printing") {
    const std::vector<std::string> types{"filter.scalar-gradient", "filter.scalar-laplacian", "filter.uv-gradient",
                                         "filter.uv-divergence", "filter.uv-vorticity"};
    for (const std::string& type : types) {
        auto a = make(type);

        // the mesh generator parameters are printed, but not for plan dumps
        const auto print  = str(*a);
        const auto custom = custom_str(*a);
        const auto name   = custom.substr(0, custom.find('['));

        EXPECT(!name.empty());
        EXPECT_EQUAL(custom, name + "[meshGeneratorParameters=...]");
        EXPECT(print.rfind(name + "[meshGeneratorParameters=", 0) == 0);
        EXPECT(print != custom);
    }
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
