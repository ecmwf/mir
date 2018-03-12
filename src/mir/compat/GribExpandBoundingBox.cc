/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/compat/GribExpandBoundingBox.h"

#include "eckit/exception/Exceptions.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Grib.h"


namespace mir {
namespace compat {


static GribExpandBoundingBox __compat("grib-expand-bounding-box");


GribExpandBoundingBox::GribExpandBoundingBox(const std::string& name) :
    GribCompatibility(name) {
}


void GribExpandBoundingBox::execute(const param::MIRParametrisation&, grib_handle*, grib_info& info) const {

    const long c = info.packing.extra_settings_count++;
    info.packing.extra_settings[c].type = GRIB_TYPE_LONG;
    info.packing.extra_settings[c].name = "expandBoundingBox";
    info.packing.extra_settings[c].long_value = 1;
}


void GribExpandBoundingBox::printParametrisation(std::ostream&, const param::MIRParametrisation&) const {
    std::ostringstream os;
    os << "GribExpandBoundingBox::printParametrisation() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


bool GribExpandBoundingBox::sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const {
    std::ostringstream os;
    os << "GribExpandBoundingBox::sameParametrisation() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void GribExpandBoundingBox::initialise(const metkit::MarsRequest&, std::map<std::string, std::string>& postproc) const {
    std::ostringstream os;
    os << "GribExpandBoundingBox::initialise() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void GribExpandBoundingBox::print(std::ostream& out) const {
    out << "GribExpandBoundingBox[]";
}


}  // namespace compat
}  // namespace mir

