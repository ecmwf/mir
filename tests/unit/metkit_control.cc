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


#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"

#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/output/GribFileOutput.h"
#include "mir/output/MIROutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/MIRStatistics.h"


namespace mir::tests::unit {


CASE("metkit control") {
    util::MIRStatistics stats;
    output::GribFileOutput out_grib("/");  // will not try to write

    for (const auto& path : {
             std::string{"mtg2.grib2"},
             std::string{"non-mtg2.grib1"},
         }) {
        input::GribFileInput in(path);
        ASSERT(in.next());

        param::SimpleParametrisation user;
        param::CombinedParametrisation param(user, dynamic_cast<input::MIRInput&>(in).parametrisation());


        // confirm the input file has MTG2Switch=true/false

        bool MTG2Switch = false;
        dynamic_cast<const param::MIRParametrisation&>(param).fieldParametrisation().get("MTG2Switch", MTG2Switch);

        std::string centre;
        dynamic_cast<const param::MIRParametrisation&>(param).fieldParametrisation().get("centre", centre);

        EXPECT(MTG2Switch == (path == "mtg2.grib2"));


        // force metkit encoder on/off with env variable (unforced behaviour depends on MTG2Switch)

        auto env = LibMir::gribUseMetkitEncoder();

        EXPECT(out_grib.do_save_with_metkit(param) ==
               (env == LibMir::UNDEFINED ? MTG2Switch && centre == "ecmf" : (env == LibMir::DEFINED_TRUE)));


        // force metkit encoder on/off with direct option

        for (const auto& forced : {true, false}) {
            user.set("grib-use-metkit-encoder", forced);
            EXPECT(out_grib.do_save_with_metkit(param) == forced);
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
