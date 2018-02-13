/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date May 2015


#include "mir/compat/ProdgenArea.h"
#include "mir/util/Grib.h"
#include "mir/param/MIRParametrisation.h"

// TODO: move to prodgen

namespace mir {
namespace compat {

ProdgenArea::ProdgenArea(const std::string& name):
    GribCompatibility(name) {

}

void ProdgenArea::execute(const param::MIRParametrisation& param, grib_handle*, grib_info& info) const {

    double d;

    switch (info.grid.grid_type) {

    case GRIB_UTIL_GRID_SPEC_REGULAR_LL:

        if (info.grid.longitudeOfLastGridPointInDegrees > 180) {
            info.grid.longitudeOfLastGridPointInDegrees -= 360;
        }

        break;


    case GRIB_UTIL_GRID_SPEC_REDUCED_GG:

        ASSERT (param.userParametrisation().get("user-east", d));
        info.grid.longitudeOfLastGridPointInDegrees = d;

        break;

    }

    info.grid.longitudeOfLastGridPointInDegrees = long(10000 * info.grid.longitudeOfLastGridPointInDegrees) / 10000.0;
    info.grid.longitudeOfFirstGridPointInDegrees = long(10000 * info.grid.longitudeOfFirstGridPointInDegrees) / 10000.0;
    info.grid.latitudeOfLastGridPointInDegrees = long(10000 * info.grid.latitudeOfLastGridPointInDegrees) / 10000.0;
    info.grid.latitudeOfFirstGridPointInDegrees = long(10000 * info.grid.latitudeOfFirstGridPointInDegrees) / 10000.0;

}

void ProdgenArea::print(std::ostream& out) const {
    out << "prodgen-areas";
}

static ProdgenArea instance("prodgen-areas");

}  // namespace method
}  // namespace mir

