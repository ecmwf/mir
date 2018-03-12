/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/compat/GribRoundToAngularPrecision.h"

#include <cmath>
#include "eckit/exception/Exceptions.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Grib.h"


namespace mir {
namespace compat {


static GribRoundToAngularPrecision __compat("grib-round-to-angular-precision");


static double round_sw(double x, double scale) {
    // round with sub-'scale' precision (under floor-truncated 'scale')
    return std::floor(x * scale + 0.1) / scale;
}


static double round_ne(double x, double scale) {
    // round with sub-'scale' precision (under ceil-truncated 'scale')
    return std::ceil(x * scale - 0.1) / scale;
}


GribRoundToAngularPrecision::GribRoundToAngularPrecision(const std::string& name) :
    GribCompatibility(name) {
}


void GribRoundToAngularPrecision::execute(const param::MIRParametrisation&, grib_handle* h, grib_info& info) const {

    // Round bounding box to GRIB accuracy (should work with ANY edition)
    long angularPrecision = 0;
    if (info.packing.editionNumber == 0) {
        GRIB_CALL(grib_get_long(h, "angularPrecision", &angularPrecision));
        ASSERT(angularPrecision > 0);
    } else if (info.packing.editionNumber == 1) {
        angularPrecision = 1000;
    } else {
        angularPrecision = 1000000;
    }
    double angularPrecisionDouble = double(angularPrecision);

    round_ne(info.grid.latitudeOfFirstGridPointInDegrees, angularPrecisionDouble);
    round_sw(info.grid.longitudeOfFirstGridPointInDegrees, angularPrecisionDouble);

    round_sw(info.grid.latitudeOfLastGridPointInDegrees, angularPrecisionDouble);
    round_ne(info.grid.longitudeOfLastGridPointInDegrees, angularPrecisionDouble);
}


void GribRoundToAngularPrecision::printParametrisation(std::ostream&, const param::MIRParametrisation&) const {
    std::ostringstream os;
    os << "GribRoundToAngularPrecision::printParametrisation() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


bool GribRoundToAngularPrecision::sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const {
    std::ostringstream os;
    os << "GribRoundToAngularPrecision::sameParametrisation() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void GribRoundToAngularPrecision::initialise(const metkit::MarsRequest&, std::map<std::string, std::string>& postproc) const {
    std::ostringstream os;
    os << "GribRoundToAngularPrecision::initialise() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void GribRoundToAngularPrecision::print(std::ostream& out) const {
    out << "GribRoundToAngularPrecision[]";
}


}  // namespace compat
}  // namespace mir

