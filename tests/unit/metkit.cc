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


#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "eckit/filesystem/TmpFile.h"
#include "eckit/testing/Test.h"

#include "mir/action/context/Context.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/output/GribFileOutput.h"
#include "mir/output/MIROutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


namespace mir::tests::unit {


enum tristate_t
{
    UNDEFINED = 0,
    FALSE,
    TRUE,
};


tristate_t use_grib_metkit_encoder_env() {
    static const std::string ENV("$MIR_GRIB_OUTPUT_METKIT");
    const long a = eckit::LibResource<long, LibMir>(ENV, 0);
    const long b = eckit::LibResource<long, LibMir>(ENV, 1);
    return a != b ? UNDEFINED : (a != 0 ? TRUE : FALSE);
}


bool use_grib_metkit_encoder(const param::MIRParametrisation& param) {
    static const auto env = use_grib_metkit_encoder_env();
    return env == UNDEFINED ? output::GribOutput::do_save_with_metkit(param) : env == TRUE;
}


CASE("metkit") {
    for (const auto& path : std::vector<std::string>{
             "gridType=reduced_gg,gridName=O8,shortName=q.grib2",
             "gridType=healpix,Nside=2,orderingConvention=nested.grib2",
             "gridType=healpix,Nside=2,orderingConvention=ring.grib2",

             "mtg2-t.grib2",
             "mtg2-vod.grib2",
             "mtg2-sfc.grib2",
             "mtg2-o2d.grib2",
             "mtg2-d.grib2",
         }) {
        Log::info() << "path: '" << path << "'" << std::endl;
        util::MIRStatistics stats;

        std::unique_ptr<input::MIRInput> in(new input::GribFileInput(path));
        ASSERT(in && in->next());

        param::SimpleParametrisation user;
        param::CombinedParametrisation param(user, in->parametrisation());

        long MTG2Encoder = 0;
        EXPECT(in->parametrisation().get("MTG2Encoder", MTG2Encoder));

        long centre = 0;
        EXPECT(in->parametrisation().get("centre", centre));

        EXPECT(MTG2Encoder == (path.find("mtg2") == 0) && (centre) == 98);

        auto env = use_grib_metkit_encoder_env();
        EXPECT(use_grib_metkit_encoder(param) == (env == UNDEFINED ? MTG2Encoder : env == TRUE));

        // tests force the metkit encoder on/off, don't cover the unforced case
        // NOTE: work in progress
        if (use_grib_metkit_encoder_env() == UNDEFINED || !MTG2Encoder) {
            continue;
        }

        eckit::TmpFile out_path;
        // auto out_path(path + ".mk.grib");
        std::unique_ptr<output::MIROutput> out(new output::GribFileOutput(out_path));

        auto field = in->field();
        context::Context ctx(field, stats);

        out->save(param, ctx);

        stats.report(Log::info());
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
