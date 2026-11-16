// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <limits>
#include <vector>

#include "mir/util/Angles.h"
#include "mir/util/Atlas.h"
#include "mir/util/Log.h"

#include "action.h"


namespace mir::action::tests {


const std::string ROTATED{"{grid: [1, 1], area: [10, -10, -10, 10], rotation: [-40, 22]}"};


MIRValuesVector values(const std::string& grid, const F& f) {
    return field(grid, f).values(0);
}


/// Vector field, components u and v
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


CASE("filter.adjust-winds-scale-cos-latitude") {
    const std::vector<std::string> grids{"{grid: [10, 10]}", "{grid: O16}", "{grid: H8}"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            auto in  = field(grid, [](double, double lon) { return 1. + lon; }, 2);
            auto out = perform("filter.adjust-winds-scale-cos-latitude", in);

            EXPECT_EQUAL(out.dimensions(), 2);

            // scaled by 1/cos(latitude), zero at the poles
            for (size_t which = 0; which < 2; ++which) {
                for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
                    const auto lat = it->pointUnrotated().lat().value();
                    const auto in_ = in.values(which)[it->index()];
                    const auto v   = out.values(which)[it->index()];

                    EXPECT(std::abs(lat) == 90. ? v == 0.
                                                : same(v, in_ / std::cos(util::degree_to_radian(lat)), 1e-9 * v));
                }
            }
        }
    }
}


CASE("filter.adjust-winds-scale-cos-latitude, properties") {
    auto a = make("filter.adjust-winds-scale-cos-latitude");

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.adjust-winds-scale-cos-latitude"})");
    EXPECT(*a == *make_checked("{type: filter.adjust-winds-scale-cos-latitude}"));
    EXPECT(*a != *make("{type: filter.adjust-winds-directions, rotation: [-40, 22]}"));
}


CASE("filter.adjust-winds-scale-cos-latitude, errors") {
    // missing values are not supported
    auto in = field("{grid: [10, 10]}", [](double lat, double) { return lat > 50 ? 9999. : lat; }, 2, 9999.);

    EXPECT_THROWS(perform("filter.adjust-winds-scale-cos-latitude", in));
}


const double R = util::Earth::radius();


double sind(double x) {
    return std::sin(util::degree_to_radian(x));
}


double cosd(double x) {
    return std::cos(util::degree_to_radian(x));
}


/// Check field component against expected values, away from the poles (where the finite volume method is less
/// accurate), within tolerance
void expect_near(const data::MIRField& field, size_t which, const F& expected, double tolerance) {
    size_t n = 0;

    for (std::unique_ptr<repres::Iterator> it(field.representation()->iterator()); it->next();) {
        const auto& p  = it->pointUnrotated();
        const auto lat = p.lat().value();

        if (std::abs(lat) < 80.) {
            const auto v = field.values(which)[it->index()];
            const auto e = expected(lat, p.lon().value());

            if (!same(v, e, tolerance) && n++ < 5) {
                Log::info() << "lat=" << lat << ": " << v << " (expected " << e << ")" << std::endl;
            }
        }
    }

    EXPECT_EQUAL(n, 0);
}


const std::string GRID{"{grid: O32}"};


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


CASE("filter.adjust-winds-*, filter.* (nabla), printing") {
    EXPECT_EQUAL(str(*make("filter.adjust-winds-scale-cos-latitude")), "AdjustWindsScaleCosLatitude[]");
    EXPECT(str(*make("{type: filter.adjust-winds-directions, rotation: [-40, 22]}"))
               .rfind("AdjustWindsDirections[rotation=", 0) == 0);

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
