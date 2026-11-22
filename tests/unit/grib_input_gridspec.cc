// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/input/GribFileInput.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Log.h"


namespace mir::tests::unit {


/// Messages of every grid type in the test data (one or more per file)
const std::vector<std::string> FILES{
    "multi-format.grib",
    "regular_ll.2-2.grib1",
    "regular_ll.2-2.grib2",
    "rotated_ll.3-3.grib1",
    "tiny-2x2.grib",
    "gridName=F1280.area=55_65_0_145.grib1",
    "gridName=F1280.area=55_65_0_145.grib2",
    "gridName=N320.area=5_75_-50_180.grib2",
    "gridType=regular_gg,gridName=F32,shortName=msl.grib1",
    "gridType=reduced_gg,gridName=O8,shortName=q.grib2",
    "gridType=reduced_ll,shortName=wsp,packingType=grid_simple.grib1",
    "gridType=healpix,Nside=4,orderingConvention=ring.grib2",
    "gridType=healpix,Nside=32,orderingConvention=nested.grib2",
    "gridType=lambert,missingValuesPresent=1.grib2",
    "gridType=lambert_azimuthal_equal_area.grib2",
    "gridType=polar_stereographic.grib1",
    "gridType=polar_stereographic.grib2",
    "MIR-553.grib2",
    "ICON.shortName=lsm.grib2",
    "stream=wave,param=swh",
    "param=t,level=1000,resol=20",
    "gridType=sh,param=t,levtype=ml,level=1.grib2",
};


/// Keys representations get (as the GRIB keys mapping), by type
const std::vector<std::string> DOUBLES{
    "north",
    "west",
    "south",
    "east",
    "west_east_increment",
    "south_north_increment",
    "south_pole_latitude",
    "south_pole_longitude",
    "south_pole_rotation_angle",
};

const std::vector<std::string> LONGS{"Ni", "Nj", "N", "Nside", "truncation"};

const std::vector<std::string> STRINGS{"gridType", "order", "orderingConvention"};


CASE("GribInput: geometry by GRIB keys and by gridSpec (MIR_GRIDSPEC_GRIB_INPUT)") {
    auto next = [](input::GribFileInput& in, bool gridspec) {
        ::setenv("MIR_GRIDSPEC_GRIB_INPUT", gridspec ? "1" : "0", 1);
        return in.next();
    };

    // the same representations, the keys of different values (more precise by gridSpec, e.g. Gaussian latitudes) noted
    std::vector<std::string> representations;
    std::vector<std::string> keys;

    for (const auto& file : FILES) {
        input::GribFileInput legacy("../data/" + file);
        input::GribFileInput gridspec("../data/" + file);

        for (size_t count = 1; next(legacy, false); ++count) {
            ASSERT(next(gridspec, true));
            const auto where = file + " #" + std::to_string(count);

            const auto& a = static_cast<const input::MIRInput&>(legacy).parametrisation();
            const auto& b = static_cast<const input::MIRInput&>(gridspec).parametrisation();

            repres::RepresentationHandle ra(repres::RepresentationFactory::build(a));
            repres::RepresentationHandle rb(repres::RepresentationFactory::build(b));
            if (ra->spec().str() != rb->spec().str()) {
                representations.push_back(where + ": " + ra->spec().str() + " != " + rb->spec().str());
            }

            auto compare = [&](const std::string& key, auto x, auto y) {
                if (a.get(key, x) && (!b.get(key, y) || x != y)) {
                    std::ostringstream oss;
                    oss << where << ": " << key << "=" << x << " != " << y;
                    keys.push_back(oss.str());
                }
            };

            for (const auto& key : DOUBLES) {
                compare(key, 0., 0.);
            }

            for (const auto& key : LONGS) {
                compare(key, 0L, 0L);
            }

            for (const auto& key : STRINGS) {
                compare(key, std::string(), std::string());
            }

            if (std::vector<long> x, y; a.get("pl", x) && (!b.get("pl", y) || x != y)) {
                keys.push_back(where + ": pl");
            }
        }

        EXPECT_NOT(next(gridspec, true));
    }

    ::unsetenv("MIR_GRIDSPEC_GRIB_INPUT");

    for (const auto& k : keys) {
        Log::info() << "key: " << k << std::endl;
    }

    for (const auto& r : representations) {
        Log::info() << "representation: " << r << std::endl;
    }

    EXPECT(representations.empty());
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
