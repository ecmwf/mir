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
/// @date Apr 2015


#include "eckit/exception/Exceptions.h"

#include "mir/util/Grib.h"

#include "mir/input/MIRInput.h"


namespace mir {
namespace input {


MIRInput::MIRInput() {
}


MIRInput::~MIRInput() {
}


grib_handle *MIRInput::gribHandle(size_t which) const {
     ASSERT(which == 0);
    static grib_handle *handle = 0;
    if (!handle) {
        handle = grib_handle_new_from_samples(0, "GRIB1");
        ASSERT(handle);
    }
    return handle;
}


bool MIRInput::next() {
    std::ostringstream os;
    MIRInput &self = *this;
    os << "MIRInput::next() not implemented for " << self;
    throw eckit::SeriousBug(os.str());
}

size_t MIRInput::copy(double *values, size_t size) const {
    std::ostringstream os;
    os << "MIRInput::copy() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


size_t MIRInput::dimensions() const {
    std::ostringstream os;
    os << "MIRInput::dimensions() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


}  // namespace input
}  // namespace mir

