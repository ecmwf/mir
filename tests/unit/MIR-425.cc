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

#include <memory>

#include "mir/action/context/Context.h"
#include "mir/action/interpolate/Gridded2RegularLL.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/method/Method.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


// define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir::tests::unit {


CASE("MIR-425") {
    auto& log = Log::info();
    auto old  = log.precision(16);

    /*
     * interpolate from 4 (i) to 9 (o) points (supersampling);
     * the nearest neighbour method should introduce 5 missing values
     *
     *   i i    ooo
     *       >> ooo
     *   i i    ooo
     */

    param::DefaultParametrisation defaults;
    util::MIRStatistics statistics;


    // input containing 9999's as valid (not missing) values
    std::unique_ptr<input::MIRInput> input(new input::GribFileInput("MIR-425.grib1"));
    ASSERT(input->next());


    SECTION("Test default interpolation, should not contain missing values") {
        param::SimpleParametrisation user;
        user.set("grid", std::vector<double>{1., 1.});

        param::CombinedParametrisation param(user, input->parametrisation(), defaults);
        std::unique_ptr<action::Action> action(new action::interpolate::Gridded2RegularLL(param));
        context::Context ctx(*input, statistics);
        action->perform(ctx);

        bool hasMissing = ctx.field().hasMissing();
        EXPECT(!hasMissing);
    }


    SECTION("Test interpolation=nn, should not contain missing values") {
        param::SimpleParametrisation user;
        user.set("grid", std::vector<double>{1., 1.});
        user.set("interpolation", "nn");

        param::CombinedParametrisation param(user, input->parametrisation(), defaults);
        std::unique_ptr<action::Action> action(new action::interpolate::Gridded2RegularLL(param));
        context::Context ctx(*input, statistics);
        action->perform(ctx);

        bool hasMissing = ctx.field().hasMissing();
        EXPECT(!hasMissing);
    }


    SECTION("Test interpolation=nn (custom), should contain missing values") {
        param::SimpleParametrisation user;
        user.set("grid", std::vector<double>{1., 1.});
        user.set("interpolation", "nn");
        user.set("nearest-method", "distance");
        user.set("distance", 1.);  // in [m], 5/9 (o) points do not have neighbours within distance

        param::CombinedParametrisation param(user, input->parametrisation(), defaults);
        std::unique_ptr<action::Action> action(new action::interpolate::Gridded2RegularLL(param));
        context::Context ctx(*input, statistics);
        action->perform(ctx);

        bool hasMissing = ctx.field().hasMissing();
        EXPECT(hasMissing);
    }


    log.precision(old);
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
