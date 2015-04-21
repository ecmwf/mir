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


#include <iostream>

#include "eckit/exception/Exceptions.h"

#include "soyuz/util/Grib.h"

#include "soyuz/input/GribMemoryInput.h"


namespace mir {
namespace input {


GribMemoryInput::GribMemoryInput(const char* message, size_t length) {
    ASSERT(handle(grib_handle_new_from_message(0, const_cast<char*>(message), length)));
}


GribMemoryInput::~GribMemoryInput() {
}


void GribMemoryInput::print(std::ostream& out) const {
    out << "GribMemoryInput[]";
}


}  // namespace input
}  // namespace mir

