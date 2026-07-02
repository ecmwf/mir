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
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/filesystem/TmpFile.h"
#include "eckit/testing/Test.h"


// #include "mir/data/MIRField.h"
// #include "mir/input/MultiDimensionalInput.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/output/MIROutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/HEALPix.h"
// #include "mir/util/Exceptions.h"
// #include "mir/util/Log.h"
#include "mir/action/context/Context.h"
#include "mir/util/MIRStatistics.h"


namespace mir::tests::unit {


#if 0
CASE("1") {
    repres::RepresentationHandle grid(new repres::HEALPix(2, "ring"));
    data::MIRField field(grid);
    MIRValuesVector values(grid->numberOfPoints());
    field.update(values, 0);

    param::SimpleParametrisation user;
    param::CombinedParametrisation param(user);

    eckit::TmpFile path;
    std::unique_ptr<output::MIROutput> out(output::MIROutputFactory::build(path, param));

    util::MIRStatistics stats;
    mir::context::Context ctx(field, stats);
    out->save(param, ctx);
}
#endif


CASE("2") {
    for (const auto& in_path : {
             // std::string{"q.grib2"},
             // std::string{"gridName=N320.area=5_75_-50_180.grib2"},
             std::string{"gridType=healpix,Nside=2,orderingConvention=nested.grib2"},
             std::string{"gridType=healpix,Nside=2,orderingConvention=ring.grib2"},
         }) {
        std::unique_ptr<input::MIRInput> in(new input::GribFileInput(in_path));
        ASSERT(in->next());


        param::SimpleParametrisation user;
        param::CombinedParametrisation param(user, in->parametrisation());

        eckit::TmpFile out_path;
        std::unique_ptr<output::MIROutput> out(output::MIROutputFactory::build(in_path + ".mk.grib", param));


        auto field = in->field();

        util::MIRStatistics stats;
        mir::context::Context ctx(field, stats);
        out->save(param, ctx);
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
