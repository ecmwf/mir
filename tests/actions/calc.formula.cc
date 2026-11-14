// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <cmath>
#include <vector>

#include "action.h"


namespace mir::tests::actions {


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


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
