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

    eckit::Log::info() << "BEFORE ProdgenArea::execute "
                       << info.grid.latitudeOfFirstGridPointInDegrees << "/"
                       << info.grid.longitudeOfFirstGridPointInDegrees << "/"
                       << info.grid.latitudeOfLastGridPointInDegrees << "/"
                       << info.grid.longitudeOfLastGridPointInDegrees <<  " grid="
                       << info.grid.grid_type
                       << " param=" << param
                       << std::endl;

    double d;
    std::ostringstream oss;

    switch (info.grid.grid_type) {

    case GRIB_UTIL_GRID_SPEC_ROTATED_LL:
    case GRIB_UTIL_GRID_SPEC_REGULAR_LL:

        if (info.grid.longitudeOfLastGridPointInDegrees > 180) {
            info.grid.longitudeOfLastGridPointInDegrees -= 360;
        }

        break;

    case GRIB_UTIL_GRID_SPEC_REGULAR_GG:
    case GRIB_UTIL_GRID_SPEC_ROTATED_GG:
        break;

    case GRIB_UTIL_GRID_SPEC_REDUCED_GG:
    case GRIB_UTIL_GRID_SPEC_REDUCED_ROTATED_GG:

        ASSERT (param.userParametrisation().get("user-east", d));
        info.grid.longitudeOfLastGridPointInDegrees = d;

        eckit::Log::info() << "AFTER ProdgenArea::execute user-east " << d << std::endl;
        break;

    case GRIB_UTIL_GRID_SPEC_SH:
        break;


    case  GRIB_UTIL_GRID_SPEC_REDUCED_LL:
        break;


    default:
        oss << "ProdgenArea::execute grid_type " << info.grid.grid_type;
        throw eckit::SeriousBug(oss.str());
        break;

    }

    info.grid.longitudeOfLastGridPointInDegrees = long(10000 * info.grid.longitudeOfLastGridPointInDegrees) / 10000.0;
    info.grid.longitudeOfFirstGridPointInDegrees = long(10000 * info.grid.longitudeOfFirstGridPointInDegrees) / 10000.0;
    info.grid.latitudeOfLastGridPointInDegrees = long(10000 * info.grid.latitudeOfLastGridPointInDegrees) / 10000.0;
    info.grid.latitudeOfFirstGridPointInDegrees = long(10000 * info.grid.latitudeOfFirstGridPointInDegrees) / 10000.0;

    eckit::Log::info() << "AFTER ProdgenArea::execute "
                       << info.grid.latitudeOfFirstGridPointInDegrees << "/"
                       << info.grid.longitudeOfFirstGridPointInDegrees << "/"
                       << info.grid.latitudeOfLastGridPointInDegrees << "/"
                       << info.grid.longitudeOfLastGridPointInDegrees <<  " grid="
                       << info.grid.grid_type
                       << std::endl;
}

void ProdgenArea::printParametrisation(std::ostream& out, const param::MIRParametrisation &param) const {

    double d;

    if (param.userParametrisation().get("user-north", d)) {
        out << ",user-north=" << d;
    }

    if (param.userParametrisation().get("user-west", d)) {
        out << ",user-west=" << d;
    }

    if (param.userParametrisation().get("user-south", d)) {
        out << ",user-south=" << d;
    }
    if (param.userParametrisation().get("user-east", d)) {
        out << ",user-east=" << d;
    }
}

bool ProdgenArea::sameParametrisation(const param::MIRParametrisation & param1,
                                      const param::MIRParametrisation & param2) const {

    double d1, d2;


    d1 = d2 = 0;

    if (param1.userParametrisation().get("user-north", d1) != param2.userParametrisation().get("user-north", d2)) {
        return false;
    }

    if (d1 != d2) {
        return false;
    }

    d1 = d2 = 0;

    if (param1.userParametrisation().get("user-west", d1) != param2.userParametrisation().get("user-west", d2)) {
        return false;
    }

    if (d1 != d2) {
        return false;
    }
    
    d1 = d2 = 0;

    if (param1.userParametrisation().get("user-south", d1) != param2.userParametrisation().get("user-south", d2)) {
        return false;
    }

    if (d1 != d2) {
        return false;
    }

    d1 = d2 = 0;

    if (param1.userParametrisation().get("user-east", d1) != param2.userParametrisation().get("user-east", d2)) {
        return false;
    }

    if (d1 != d2) {
        return false;
    }

    return true;
}



void ProdgenArea::print(std::ostream & out) const {
    out << "prodgen-areas";
}

static ProdgenArea instance("prodgen-areas");

}  // namespace method
}  // namespace mir

