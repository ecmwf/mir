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

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "eckit/types/FloatCompare.h"

#include "mir/action/context/Context.h"
#include "mir/action/plan/Action.h"
#include "mir/api/MIRJob.h"
#include "mir/input/MIRInput.h"
#include "mir/output/RawOutput.h"
#include "mir/param/SimpleParametrisation.h"
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


CASE("Formula (pgen integration)") {
    // pgen in production uses (integration test, however this list is not extensive):
    // - use options like a=b=c (CmdArgs cannot use this, a CmdArgs::init parsing problem)
    // - use options like a.b=c (CmdArgs cannot use this, because of LocalConfiguration default separator '.')
    // - use formula with/without associated metadata

    // setup input/output/job
    param::SimpleParametrisation in;
    in.set("input",
           "{"
           "artificialInput:constant,"
           "constant:1.,"
           "gridded:true,"
           "gridType:regular_ll,"
           "west_east_increment:1.,"
           "south_north_increment:1.,"
           "Ni:360,"
           "Nj:181,"
           "north:90.,"
           "west:0.,"
           "south:-90.,"
           "east:360."
           "}");

    std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build("constant", in));

    param::SimpleParametrisation out;
    std::vector<double> values(7320, 0.);
    std::unique_ptr<output::MIROutput> output(new output::RawOutput(values.data(), values.size(), out));

    api::MIRJob job;
    job.set("grid", std::vector<double>{3, 3});
    job.set("formula.prologue", "3-f");
    job.set("formula.epilogue", "f1*3");
    job.set("formula.epilogue.metadata", "paramId=255");

    job.execute(*input, *output);

    auto is_approximately_equal = [](const std::vector<double>& values) -> bool {
        return values.end() == std::adjacent_find(values.begin(), values.end(), [](double a, double b) {
                   return !eckit::types::is_approximately_equal(a, b);
               });
    };

    // (3 - 1) * 3 = 6
    EXPECT(is_approximately_equal(values));
    EXPECT_APPROX(6., values[0]);
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
