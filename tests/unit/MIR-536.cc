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
#include "mir/grib/Packing.h"
#include "mir/output/GribMemoryOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tests::unit {


CASE("MIR-536") {
    std::unique_ptr<output::MIROutput> out(new output::GribMemoryOutput(nullptr, 0));

    struct Param : param::SimpleParametrisation {
        Param() { field_.set("edition", 0L).set("accuracy", 0L).set("packing", "not_simple").set("gridded", true); }
        const MIRParametrisation& userParametrisation() const override { return *this; }
        const MIRParametrisation& fieldParametrisation() const override { return field_; }

    private:
        SimpleParametrisation field_;
    };


    SECTION("Defaults") {
        Param defaults;

        std::unique_ptr<action::Action> a(new action::io::Save(defaults, *out));
        std::unique_ptr<action::Action> b(new action::io::Save(defaults, *out));

        EXPECT(a->sameAs(*b));
    }


    SECTION("Packing changes") {
        Param param[4];
        param[1].set("packing", "simple");
        param[2].set("accuracy", 12L);
        param[3].set("edition", 2L);

        for (size_t i = 0; i < 4; ++i) {
            for (size_t j = 0; j < 4; ++j) {
                std::unique_ptr<action::Action> a(new action::io::Save(param[i], *out));
                std::unique_ptr<action::Action> b(new action::io::Save(param[j], *out));

                if (i == j) {
                    EXPECT(a->sameAs(*b));
                }
                else {
                    EXPECT_NOT(a->sameAs(*b));
                }
            }
        }
    }


    SECTION("Control grib-edition-conversion") {
        std::unique_ptr<grib::Packing> ptr;

        Param param;
        param.set("packing", "ccsds");  // edition=2 only

        param.set("grib-edition-conversion", false);
        EXPECT_THROWS_AS(ptr.reset(grib::Packing::build(param)), exception::UserError);

        param.set("grib-edition-conversion", true);
        EXPECT_NO_THROW(ptr.reset(grib::Packing::build(param)));
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
