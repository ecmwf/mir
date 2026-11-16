// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <string>
#include <vector>

#include "mir/util/Angles.h"

#include "action.h"


namespace mir::action::tests {


std::string formula(const std::string& formula, const std::string& metadata = "") {
    return "{type: calc.formula, formula: '" + formula + "', formula.metadata: '" + metadata + "'}";
}


CASE("calc.formula") {
    struct test_t {
        std::string formula;
        std::function<double(double)> f;
    };

    auto in = field("{grid: [10, 10]}", [](double lat, double) { return lat; }, 2);

    const std::vector<test_t> tests{
        test_t{"f", [](double x) { return x; }},
        test_t{"f * 2 + 1", [](double x) { return x * 2 + 1; }},
        test_t{"1 - f / 2", [](double x) { return 1 - x / 2; }},
        test_t{"-f", [](double x) { return -x; }},
        test_t{"(f + 1) * (f - 1)", [](double x) { return (x + 1) * (x - 1); }},
        test_t{"f ^ 2", [](double x) { return x * x; }},
        test_t{"pow(f, 2)", [](double x) { return x * x; }},
        test_t{"sqrt(abs(f))", [](double x) { return std::sqrt(std::abs(x)); }},
        test_t{"max(f, 0)", [](double x) { return std::max(x, 0.); }},
        test_t{"min(0, f)", [](double x) { return std::min(0., x); }},
        test_t{"f > 0", [](double x) { return x > 0 ? 1. : 0.; }},
        test_t{"f <= 0", [](double x) { return x <= 0 ? 1. : 0.; }},
        test_t{"f + 2 * 3", [](double x) { return x + 6; }},
        test_t{"1.5e1 + f", [](double x) { return 15 + x; }},
    };
    for (const auto& test : tests) {
        SECTION(test.formula) {
            auto out = perform(formula(test.formula), in);

            EXPECT_EQUAL(out.dimensions(), 2);

            for (size_t which = 0; which < 2; ++which) {
                const auto& v = out.values(which);

                EXPECT_EQUAL(v.size(), in.values(which).size());

                for (size_t i = 0; i < v.size(); ++i) {
                    EXPECT(same(v[i], test.f(in.values(which)[i])));
                }
            }
        }
    }
}


CASE("calc.formula, field components") {
    auto in = field("{grid: [10, 10]}", [](double lat, double) { return lat; }, 2);
    MIRValuesVector two(in.values(1).size(), 2.);
    in.update(two, 1);

    auto out = perform(formula("f1 - f2"), in);

    EXPECT_EQUAL(out.dimensions(), 1);

    for (size_t i = 0; i < out.values(0).size(); ++i) {
        EXPECT(same(out.values(0)[i], in.values(0)[i] - 2.));
    }
}


CASE("calc.formula, metadata") {
    auto in  = field("{grid: [10, 10]}", [](double lat, double) { return lat; }, 2);
    auto out = perform(formula("f * 2", "paramId=129,level=1"), in);

    for (size_t which = 0; which < 2; ++which) {
        const auto& md = out.metadata(which);

        EXPECT_EQUAL(md.size(), 2);
        EXPECT_EQUAL(md.at("paramId"), 129);
        EXPECT_EQUAL(md.at("level"), 1);
    }
}


CASE("calc.formula, with missing values") {
    auto in = field("{grid: [10, 10]}", [](double lat, double) { return lat > 50 ? 9999. : lat; }, 1, 9999.);

    const std::vector<std::string> all_values{"f * 2", "2 * f", "f + f", "sqrt(abs(f))"};
    for (const std::string& f : all_values) {
        SECTION(f) {
            auto out = perform(formula(f), in);

            EXPECT(out.hasMissing());
            EXPECT(out.missingValue() == 9999.);
            EXPECT_EQUAL(count_missing(out), count_missing(in));
        }
    }
}


CASE("calc.formula, comparison") {
    auto a = make(formula("f * 2", "paramId=129"));

    EXPECT(*a == *make(formula("f*2", "paramId=129")));
    EXPECT(*a != *make(formula("f * 3", "paramId=129")));
    EXPECT(*a != *make(formula("2 * f", "paramId=129")));
    EXPECT(*a != *make(formula("f * 2", "paramId=130")));
    EXPECT(*a != *make(formula("f * 2")));
    EXPECT(*make(formula("sqrt(f)")) != *make(formula("abs(f)")));
    EXPECT(*make(formula("f1")) != *make(formula("f2")));
}


CASE("calc.formula, errors") {
    EXPECT_THROWS(make("calc.formula"));                                  // formula is required
    EXPECT_THROWS(make("{type: calc.formula, formula: f}"));              // formula.metadata is required
    EXPECT_THROWS(make(formula("f +")));                                  // syntax
    EXPECT_THROWS(make(formula("f * (2")));                               // syntax
    EXPECT_THROWS(make(formula("unknown(f)")));                           // function
    EXPECT_THROWS(make(formula("f", "paramId")));                         // metadata syntax
    EXPECT_THROWS(perform(formula("g * 2"), field("{grid: [10, 10]}")));  // variable
}


CASE("misc.checkerboard") {
    const std::vector<std::string> grids{"{grid: [10, 10]}", "{grid: O16}", "{grid: H8}"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            auto in  = field(grid, [](double lat, double) { return lat; }, 2);
            auto out = perform(*make("misc.checkerboard"), in);

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), in.values(0).size());
            EXPECT(!out.hasMissing());

            for (size_t which = 0; which < 2; ++which) {
                const auto& v = out.values(which);

                EXPECT(std::set<double>(v.begin(), v.end()) == std::set<double>({0., 1.}));
            }
        }
    }
}


CASE("misc.checkerboard, frequencies") {
    // four boxes (two in longitude, two in latitude), alternating
    auto in  = field("{grid: [10, 10]}", [](double lat, double) { return lat; });
    auto out = perform(*make("{type: misc.checkerboard, frequencies: [2, 2], 0-1: true}"), in);

    for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
        const auto& p  = it->pointUnrotated();
        const auto lat = p.lat().value();
        const auto lon = p.lon().normalise(Longitude::GREENWICH).value();

        if (lat > -90.) {
            const bool east  = lon >= 180.;
            const bool south = lat <= 0.;

            EXPECT(out.values(0)[it->index()] == (east != south ? 1. : 0.));
        }
    }
}


CASE("misc.checkerboard, with missing values") {
    SECTION("missing values are kept") {
        auto in  = field("{grid: [10, 10]}", [](double lat, double) { return lat > 50 ? 9999. : lat; }, 1, 9999.);
        auto out = perform(*make("misc.checkerboard"), in);

        EXPECT(out.hasMissing());
        EXPECT_EQUAL(count_missing(out), count_missing(in));

        for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
            const auto v = out.values(0)[it->index()];

            EXPECT(it->pointUnrotated().lat().value() > 50 ? v == 9999. : (v == 0. || v == 1.));
        }
    }

    SECTION("only missing values: unchanged") {
        auto in  = field("{grid: [10, 10]}", [](double, double) { return 9999.; }, 1, 9999.);
        auto out = perform(*make("misc.checkerboard"), in);

        EXPECT(out.values(0) == in.values(0));
    }
}


CASE("misc.checkerboard, properties") {
    auto a = make("{type: misc.checkerboard, frequencies: [2, 2], 0-1: true}");
    auto b = make(a->json_str(false));

    EXPECT_EQUAL(b->json_str(false), a->json_str(false));
    EXPECT_EQUAL(a->json_str(), R"({"type":"misc.checkerboard","0-1":true,"frequencies":[2,2]})");
}


/// Expected pattern value, for the (non-missing) range of input values
double pattern(double lat, double lon, double min, double max, double f1, double f2) {
    return (max - min) * 0.5 * std::sin(f1 / 2. * util::degree_to_radian(lon)) *
               std::cos(f2 * util::degree_to_radian(lat)) +
           (min + max) / 2.;
}


CASE("misc.pattern") {
    struct test_t {
        std::string spec;
        bool normalise;
        double f1;
        double f2;
    };

    const std::vector<std::string> grids{"{grid: [10, 10]}", "{grid: O16}", "{grid: H8}"};
    for (const auto& grid : grids) {
        const std::vector<test_t> tests{
            test_t{"misc.pattern", false, 6., 3.},
            test_t{"{type: misc.pattern, 0-1: true}", true, 6., 3.},
            test_t{"{type: misc.pattern, frequencies: [2, 1]}", false, 2., 1.},
        };
        for (const auto& test : tests) {
            SECTION(std::string(grid) + " " + test.spec) {
                auto in  = field(grid, [](double lat, double) { return lat; }, 2);
                auto out = perform(*make(test.spec), in);

                const auto& v0 = in.values(0);
                const auto min = test.normalise ? 0. : *std::min_element(v0.begin(), v0.end());
                const auto max = test.normalise ? 1. : *std::max_element(v0.begin(), v0.end());

                EXPECT_EQUAL(out.dimensions(), 2);
                EXPECT(!out.hasMissing());

                for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
                    const auto& p = it->pointUnrotated();
                    const auto v  = pattern(p.lat().value(), p.lon().value(), min, max, test.f1, test.f2);

                    EXPECT(same(out.values(0)[it->index()], v));
                    EXPECT(same(out.values(1)[it->index()], v));
                }
            }
        }
    }
}


CASE("misc.pattern, with missing values") {
    SECTION("missing values are kept, range of non-missing values") {
        auto in  = field("{grid: [10, 10]}", [](double lat, double) { return lat > 50 ? 9999. : lat; }, 1, 9999.);
        auto out = perform(*make("misc.pattern"), in);

        EXPECT(out.hasMissing());
        EXPECT_EQUAL(count_missing(out), count_missing(in));

        for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
            const auto& p = it->pointUnrotated();
            const auto v  = out.values(0)[it->index()];

            EXPECT(p.lat().value() > 50 ? v == 9999.
                                        : same(v, pattern(p.lat().value(), p.lon().value(), -90., 50., 6., 3.)));
        }
    }

    SECTION("only missing values: unchanged") {
        auto in  = field("{grid: [10, 10]}", [](double, double) { return 9999.; }, 2, 9999.);
        auto out = perform(*make("misc.pattern"), in);

        EXPECT(out.values(0) == in.values(0));
        EXPECT(out.values(1) == in.values(1));
    }
}


CASE("misc.pattern, properties") {
    auto a = make("{type: misc.pattern, frequencies: [2, 1], 0-1: true}");
    auto b = make(a->json_str(false));

    EXPECT_EQUAL(b->json_str(false), a->json_str(false));
    EXPECT_EQUAL(a->json_str(), R"({"type":"misc.pattern","0-1":true,"frequencies":[2,1]})");
}


CASE("misc.reset-missing-values") {
    const auto lowest = std::numeric_limits<double>::lowest();

    SECTION("first value repeated: missing values") {
        auto in  = field("{grid: [10, 10]}", [](double lat, double) { return lat > 0 ? 1234. : lat; });
        auto out = perform("misc.reset-missing-values", in);

        EXPECT(out.hasMissing());
        EXPECT(out.missingValue() == lowest);
        EXPECT(count_missing(out) > 0);
        EXPECT(count_missing(out) < out.values(0).size());
    }

    SECTION("constant field: no missing values") {
        auto out = perform("misc.reset-missing-values", field("{grid: [10, 10]}", [](double, double) { return 1.; }));

        EXPECT(!out.hasMissing());
    }

    SECTION("field with missing values: unchanged") {
        auto in = field("{grid: [10, 10]}", [](double lat, double) { return lat > 0 ? 9999. : lat; }, 1, 9999.);

        EXPECT(in.hasMissing());

        auto out = perform("misc.reset-missing-values", in);

        EXPECT(out.missingValue() == 9999.);
        EXPECT(out.values(0) == in.values(0));
    }
}


CASE("noop") {
    auto in  = field("{grid: [10, 10]}", [](double lat, double lon) { return lat + lon; });
    auto out = perform("noop", in);

    EXPECT(out.representation()->sameAs(*in.representation()));
    EXPECT(out.values(0) == in.values(0));
}


CASE("noop, comparison") {
    EXPECT(*make("noop") == *make("{type: noop}"));
    EXPECT(*make("noop") != *make("misc.reset-missing-values"));
}


CASE("select.field") {
    auto in = field("{grid: [10, 10]}");

    for (size_t which = 0; which < 3; ++which) {
        MIRValuesVector values(in.values(0).size(), static_cast<double>(which));
        in.update(values, which);
    }

    EXPECT_EQUAL(in.dimensions(), 3);

    for (size_t which = 0; which < 3; ++which) {
        SECTION("which: " + std::to_string(which)) {
            auto out = perform("{type: select.field, which: " + std::to_string(which) + "}", in);

            EXPECT_EQUAL(out.dimensions(), 1);
            EXPECT(out.values(0).front() == static_cast<double>(which));
        }
    }
}


CASE("select.field, comparison and errors") {
    EXPECT(*make("{type: select.field, which: 1}") != *make("{type: select.field, which: 0}"));
    EXPECT_THROWS(make("select.field"));  // which is required
}


CASE("set.metadata") {
    auto out = perform("{type: set.metadata, metadata: 'paramId=167,level=2'}", field("{grid: [10, 10]}", F{}, 2));

    for (size_t which = 0; which < out.dimensions(); ++which) {
        const auto& md = out.metadata(which);

        EXPECT_EQUAL(md.at("paramId"), 167);
        EXPECT_EQUAL(md.at("level"), 2);
    }
}


CASE("set.metadata, comparison and errors") {
    EXPECT(*make("{type: set.metadata, metadata: paramId=1}") != *make("{type: set.metadata, metadata: paramId=2}"));
    EXPECT_THROWS(make("{type: set.metadata, metadata: paramId}"));
}


CASE("calc.formula, misc.*, noop, select.field, set.metadata, printing") {
    const auto formula = str(*make("{type: calc.formula, formula: 'f + 1', formula.metadata: 'paramId=1'}"));

    EXPECT(formula.rfind("FormulaAction[", 0) == 0);
    EXPECT(formula.find(", metadata=") != std::string::npos);

    EXPECT_EQUAL(str(*make("noop")), "Noop[]");
    EXPECT_EQUAL(str(*make("{type: select.field, which: 1}")), "SelectField[1]");
    EXPECT(str(*make("{type: set.metadata, metadata: paramId=1}")).rfind("SetMetadata[", 0) == 0);
    EXPECT_EQUAL(custom_str(*make("noop")), "Noop[]");
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
