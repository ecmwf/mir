// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/compat/MillidegreeArea.h"

#include <cmath>
#include <cstring>
#include <ostream>
#include <string>
#include <vector>

#include "mir/output/MIROutput.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir::compat {


static const std::string NAME{"millidegree-area"};
static const MillidegreeArea INSTANCE(NAME);

static const std::vector<std::string> USER_AREA{"user-north", "user-west", "user-south", "user-east"};


// to the millidegree, symmetrically about 0, and short of 360 (as prodgen's grib.c)
static double round_millidegree(double x) {
    if (359.999 < x && x < 360.) {
        x = 359.999;
    }

    constexpr double PRECISION = 1000.;
    return x < 0 ? -std::round(-x * PRECISION) / PRECISION : std::round(x * PRECISION) / PRECISION;
}


void MillidegreeArea::execute(const output::MIROutput& output, const param::MIRParametrisation& param, grib_handle* h,
                              grib_info& info) const {
    // the edition as the input's, unless requested; not global nor the bounding box expanded (the area as given)
    long edition = 0;
    codes_get_long(h, "edition", &edition);

    for (long j = 0; j < info.packing.extra_settings_count; j++) {
        auto& set = info.packing.extra_settings[j];
        if (std::strcmp(set.name, "edition") == 0) {
            edition = set.long_value;
        }
        if (std::strcmp(set.name, "global") == 0 || std::strcmp(set.name, "expandBoundingBox") == 0) {
            set.long_value = 0;
        }
    }

    ASSERT(edition == 1 || edition == 2);

    auto& grid = info.grid;
    const double before[]{grid.latitudeOfFirstGridPointInDegrees, grid.longitudeOfFirstGridPointInDegrees,
                          grid.latitudeOfLastGridPointInDegrees, grid.longitudeOfLastGridPointInDegrees};

    switch (grid.grid_type) {
        case CODES_UTIL_GRID_SPEC_ROTATED_LL:
        case CODES_UTIL_GRID_SPEC_REGULAR_LL:
            if (grid.longitudeOfFirstGridPointInDegrees <= -180) {
                grid.longitudeOfFirstGridPointInDegrees += 360;
            }
            if (grid.longitudeOfLastGridPointInDegrees > 180) {
                grid.longitudeOfLastGridPointInDegrees -= 360;
            }
            break;

        case CODES_UTIL_GRID_SPEC_REDUCED_GG:
        case CODES_UTIL_GRID_SPEC_REDUCED_ROTATED_GG:
            if (grid.longitudeOfFirstGridPointInDegrees < 0) {
                grid.longitudeOfFirstGridPointInDegrees += 360;
            }
            if (double east = 0; param.userParametrisation().get("user-east", east)) {
                grid.longitudeOfLastGridPointInDegrees = east;
            }
            if (grid.longitudeOfLastGridPointInDegrees < 0) {
                grid.longitudeOfLastGridPointInDegrees += 360;
            }
            break;

        case CODES_UTIL_GRID_SPEC_REGULAR_GG:
        case CODES_UTIL_GRID_SPEC_ROTATED_GG:
        case CODES_UTIL_GRID_SPEC_SH:
        case CODES_UTIL_GRID_SPEC_REDUCED_LL:
            break;

        default:
            throw exception::SeriousBug("MillidegreeArea: unsupported grid_type " + std::to_string(grid.grid_type));
    }

    if (edition == 2) {
        if (grid.longitudeOfFirstGridPointInDegrees < 0) {
            grid.longitudeOfFirstGridPointInDegrees += 360;
        }
        if (grid.longitudeOfLastGridPointInDegrees < 0) {
            grid.longitudeOfLastGridPointInDegrees += 360;
        }
    }

    grid.latitudeOfFirstGridPointInDegrees  = round_millidegree(grid.latitudeOfFirstGridPointInDegrees);
    grid.longitudeOfFirstGridPointInDegrees = round_millidegree(grid.longitudeOfFirstGridPointInDegrees);
    grid.latitudeOfLastGridPointInDegrees   = round_millidegree(grid.latitudeOfLastGridPointInDegrees);
    grid.longitudeOfLastGridPointInDegrees  = round_millidegree(grid.longitudeOfLastGridPointInDegrees);

    if (before[0] != grid.latitudeOfFirstGridPointInDegrees || before[1] != grid.longitudeOfFirstGridPointInDegrees ||
        before[2] != grid.latitudeOfLastGridPointInDegrees || before[3] != grid.longitudeOfLastGridPointInDegrees) {
        Log::info() << "MillidegreeArea " << output << " grid=" << grid.grid_type << ", edition=" << edition
                    << " BEFORE " << before[0] << "/" << before[1] << "/" << before[2] << "/" << before[3] << " AFTER "
                    << grid.latitudeOfFirstGridPointInDegrees << "/" << grid.longitudeOfFirstGridPointInDegrees << "/"
                    << grid.latitudeOfLastGridPointInDegrees << "/" << grid.longitudeOfLastGridPointInDegrees
                    << std::endl;
    }
}


void MillidegreeArea::printParametrisation(std::ostream& out, const param::MIRParametrisation& param) const {
    for (const auto& key : USER_AREA) {
        if (double d = 0; param.userParametrisation().get(key, d)) {
            out << "," << key << "=" << d;
        }
    }
}


bool MillidegreeArea::sameParametrisation(const param::MIRParametrisation& param1,
                                          const param::MIRParametrisation& param2) const {
    for (const auto& key : USER_AREA) {
        double d1 = 0;
        double d2 = 0;
        if (param1.userParametrisation().get(key, d1) != param2.userParametrisation().get(key, d2) || d1 != d2) {
            return false;
        }
    }
    return true;
}


// the user area keys (user-north, ...) are set by the client from its request's area (mir doesn't depend on metkit
// to read it), e.g. as postproc["user-north"] = request.values("area")[0]
void MillidegreeArea::initialise(const metkit::mars::MarsRequest&, std::map<std::string, std::string>&) const {}


void MillidegreeArea::print(std::ostream& out) const {
    out << NAME;
}


}  // namespace mir::compat
