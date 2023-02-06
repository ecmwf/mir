/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "eckit/testing/Test.h"

#include <cmath>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "eckit/types/FloatCompare.h"

#include "mir/action/context/Context.h"
#include "mir/action/plan/Action.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Formula.h"
#include "mir/util/FormulaParser.h"
#include "mir/util/Log.h"

#define EXPECT_APPROX(a, b)                                                                                        \
    EXPECT_MSG(eckit::types::is_approximately_equal(a, b, 1e-12), [=]() {                                          \
        std::cerr << eckit::Colour::red << "FAILED " << #a " == " << #b << " evaluated as [" << a << "] == [" << b \
                  << "]" << eckit::Colour::reset << std::endl;                                                     \
    };)


namespace mir {
namespace tests {
namespace unit {


CASE("Formula") {
    auto eval = [](const std::string& formula) {
        context::Context ctx;
        ctx.scalar(0.);

        std::istringstream iss(formula);
        iss >> std::setprecision(16);
        util::FormulaParser parser(iss);

        static const param::SimpleParametrisation empty;
        std::unique_ptr<action::Action> action(parser.parse(empty));
        action->perform(ctx);

        return ctx.scalar();
    };

    auto s = [](double d) {
        std::ostringstream str;
        str << std::setprecision(32) << d;
        return str.str();
    };

    using t = std::pair<std::string, double>;

    SECTION("f(scalar)") {
        for (const auto& test : {
                 // clang-format off
                 t("abs(1)", 1),
                 t("abs(-1)", 1),
                 t("neg(1)", -1),
                 t("neg(-1)", 1),
                 t("not(1)", 0),
                 t("not(-1)", 0),
                 t("not(0)", 1),
                 t("sin(0)", 0),
                 t("sin(" + s(M_PI_2  / 3.) + ")", 1. / 2.),
                 t("sin(" + s(M_PI_4) + ")", std::sqrt(2.) / 2.),
                 t("sin(" + s(M_PI / 3.) + ")", std::sqrt(3.) / 2.),
                 t("sin(" + s(M_PI_2) + ")", 1.),
                 t("cos(0)", 1.),
                 t("cos(" + s(M_PI_2  / 3.) + ")", std::sqrt(3.) / 2.),
                 t("cos(" + s(M_PI_4) + ")", std::sqrt(2.) / 2.),
                 t("cos(" + s(M_PI / 3.) + ")", 1. / 2.),
                 t("cos(" + s(M_PI_2) + ")", 0.),
                 t("tan(0)", 0.),
                 t("tan(" + s(M_PI_2  / 3.) + ")", std::sqrt(3.) / 3.),
                 t("tan(" + s(M_PI_4) + ")", 1.),
                 t("tan(" + s(M_PI / 3.) + ")", std::sqrt(3.)),
                 t("asin(0)", 0.),
                 t("asin(" + s(1. / 2.) + ")", M_PI_2  / 3.),
                 t("asin(" + s(std::sqrt(2.) / 2.) + ")", M_PI_4),
                 t("asin(" + s(std::sqrt(3.) / 2.) + ")", M_PI / 3.),
                 t("asin(1)", M_PI_2),
                 t("acos(1)", 0.),
                 t("acos(" + s(std::sqrt(3.) / 2.) + ")", M_PI_2  / 3.),
                 t("acos(" + s(std::sqrt(2.) / 2.) + ")", M_PI_4),
                 t("acos(" + s(1. / 2.) + ")", M_PI / 3.),
                 t("acos(0)", M_PI_2),
                 t("atan(0)", 0.),
                 t("atan(" + s(std::sqrt(3.) / 3.) + ")", M_PI_2  / 3.),
                 t("atan(1)", M_PI_4),
                 t("atan(" + s(std::sqrt(3.)) + ")", M_PI / 3.),
                 t("exp(0)", 1.),
                 t("exp(1)", std::exp(1.)),
                 t("log(1)", 0.),
                 t("log(" + s(std::exp(1.)) + ")", 1.),
                 t("log(" + s(std::exp(2.)) + ")", 2.),
                 t("log10(1)", 0),
                 t("log10(10)", 1),
                 t("log10(100)", 2),
                 t("log2(1)", 0),
                 t("log2(2)", 1),
                 t("log2(4)", 2),
                 t("round(1.1)", 1),
                 t("round(1.9)", 2),
                 t("round(-1.1)", -1),
                 t("round(-1.9)", -2),
                 t("sqrt(1)", 1),
                 t("sqrt(4)", 2),
                 // clang-format on
             }) {
            break;

            Log::info() << test.first << " -> " << eval(test.first) << " ~= " << test.second << std::endl;
            EXPECT_APPROX(eval(test.first), test.second);
        }
    }


    SECTION("f(scalar, scalar)") {
        for (const auto& test : {
                 // clang-format off
                 t("atan2(0, 1)", 0),
                 t("atan2(1,1)", M_PI_4),
                 t("atan2(-1,1)", -M_PI_4),
                 t("atan2(1,-1)", 3. * M_PI_4),
                 t("atan2(-1,-1)", -3. * M_PI_4),
                 t("max(1,-1)", 1),
                 t("max(-1,1)", 1),
                 t("min(1,-1)", -1),
                 t("min(-1,1)", -1),
                 t("pow(1,1)", 1),
                 t("pow(1,2)", 1),
                 t("pow(2,1)", 2),
                 t("pow(2,2)", 4),
                 t("pow(2,-1)", 1. / 2.),
                 t("pow(2,-2)", 1. / 4.),
                 t("1 * 2", 2),
                 t("1 + 2", 3),
                 t("1 - 2", -1),
                 t("1 / 2", 0.5),
                 t("1 < 2", 1),
                 t("2 < 1", 0),
                 t("1 < 1", 0),
                 t("1 > 2", 0),
                 t("2 > 1", 1),
                 t("1 > 1", 0),
                 t("1 <= 2", 1),
                 t("2 <= 1", 0),
                 t("1 <= 1", 1),
                 t("1 >= 2", 0),
                 t("2 >= 1", 1),
                 t("1 >= 1", 1),
                 t("1 = 1", 1),
                 t("1 = 2", 0),
                 t("and(1,0)", 0),
                 t("and(1,1)", 1),
                 t("and(1,-1)", 1),
                 t("and(0,0)", 0),
                 t("and(0,1)", 0),
                 t("and(0,-1)", 0),
                 t("or(1,0)", 1),
                 t("or(1,1)", 1),
                 t("or(1,-1)", 1),
                 t("or(0,0)", 0),
                 t("or(0,1)", 1),
                 t("or(0,-1)", 1),
#if 0
                t("x != y", 1),
                t("x && y", 0),
                t("x ^ y", 0),
                t("1 || y", 0),
#endif
                 // clang-format on
             }) {
            Log::info() << test.first << " -> " << eval(test.first) << " ~= " << test.second << std::endl;
            EXPECT_APPROX(eval(test.first), test.second);
        }
    }
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
