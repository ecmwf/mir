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
#include <string>

#include "eckit/testing/Test.h"

#include "mir/input/GribFileInput.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tests::unit {


CASE("GribFileInput") {
    SECTION("gridded") {
        for (const std::string& path : {
                 "../data/ICON.shortName=lsm.grib2",
                 "../data/MIR-375.grib1",
                 "../data/MIR-553.grib2",
                 "../data/O400.version=1.grib1",
                 "../data/O400.version=2.grib1",
                 "../data/ORCA.grib2",
                 "../data/gridType=healpix,Nside=32,orderingConvention=ring.grib2",
                 "../data/gridType=reduced_gg,gridName=N320,shortName=msl.grib1",
                 "../data/gridType=reduced_gg,gridName=O1280,shortName=msl.grib1",
                 "../data/gridType=reduced_gg,gridName=O32,shortName=msl.grib1",
                 "../data/gridType=reduced_ll,shortName=2dfd,packingType=grid_simple_matrix.grib1",
                 "../data/gridType=reduced_ll,shortName=wsp,packingType=grid_simple.grib1",
                 "../data/gridType=regular_gg,gridName=F32,shortName=msl.grib1",
                 "../data/gridType=regular_gg,gridName=F48,shortName=msl,global=0.001.grib1",
                 "../data/gridType=regular_gg,gridName=F48,shortName=msl,global=0.002.grib1",
                 "../data/gridType=regular_gg,gridName=F640,shortName=msl,global=0.grib1",
                 "../data/gridType=regular_gg,gridName=F80,shortName=msl,global=0.001.grib1",
                 "../data/gridType=regular_gg,gridName=F80,shortName=msl,global=0.002.grib1",
                 "../data/levtype=ml,levelist=1,param=lnsp,grid=O32",
                 "../data/mofc_ocean_fc_inst_horizontal.grib1",
                 "../data/orog_1km.grib2",
                 "../data/packingType=grid_ccsds.grib2",
                 "../data/packingType=grid_complex.grib2",
                 "../data/packingType=grid_ieee.grib2",
                 "../data/packingType=grid_second_order.grib2",
                 "../data/packingType=grid_simple.grib1",
                 "../data/packingType=grid_simple.grib2",
                 "../data/regular_ll.2-2.grib1",
                 "../data/regular_ll.2-2.grib2",
                 "../data/stream=wave,param=2dfd,direction=1,frequency=1",
             }) {
            Log::info() << "Testing '" << path << "'..." << std::endl;
            std::unique_ptr<input::MIRInput> input(new input::GribFileInput(path));

            // testing first message only
            ASSERT(input->next());
            const auto& param = input->parametrisation();

            auto gridded_1 = param.has("gridded");
            EXPECT(gridded_1);

            auto gridded_2 = false;
            EXPECT(param.get("gridded", gridded_2));

            EXPECT_EQUAL(gridded_1, gridded_2);
        }
    }


    SECTION("spectral") {
        for (const std::string& path : {
                 "../data/date=19930202",
                 "../data/gridType=sh,shortName=t,J=20.grib1",
                 "../data/packingType=spectral_complex,param=vo_d.grib1",
                 "../data/packingType=spectral_complex,param=vo_d.grib2",
                 "../data/packingType=spectral_complex.grib1",
                 "../data/packingType=spectral_complex.grib2",
                 "../data/packingType=spectral_simple.grib1",
             }) {
            Log::info() << "Testing '" << path << "'..." << std::endl;
            std::unique_ptr<input::MIRInput> input(new input::GribFileInput(path));

            // testing first message only
            ASSERT(input->next());
            const auto& param = input->parametrisation();

            auto spectral_1 = param.has("spectral");
            EXPECT(spectral_1);

            auto spectral_2 = false;
            EXPECT(param.get("spectral", spectral_2));

            EXPECT_EQUAL(spectral_1, spectral_2);
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
