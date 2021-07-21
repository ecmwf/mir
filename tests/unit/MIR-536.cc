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


#include <memory>

#include "eckit/testing/Test.h"

#include "mir/action/io/Save.h"
#include "mir/input/GribFileInput.h"
#include "mir/output/GribFileOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"


namespace mir {
namespace tests {
namespace unit {


CASE("MIR-536") {
    std::unique_ptr<input::MIRInput> in(new input::GribFileInput(""));
    std::unique_ptr<output::MIROutput> out(new output::GribFileOutput(""));

    struct Param : param::SimpleParametrisation {
        const MIRParametrisation& userParametrisation() const override { return *this; }
        const MIRParametrisation& fieldParametrisation() const override { return *this; }
    };


    SECTION("Defaults") {
        Param defaults;

        std::unique_ptr<action::Action> a(new action::io::Save(defaults, *in, *out));
        std::unique_ptr<action::Action> b(new action::io::Save(defaults, *in, *out));

        EXPECT(a->sameAs(*b));
    }
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
