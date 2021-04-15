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


#include "mir/input/GribMemoryInput.h"

#include <ostream>

#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir {
namespace input {


GribMemoryInput::GribMemoryInput(const void* message, size_t length) {
    GRIB_CALL(codes_check_message_header(message, length, PRODUCT_GRIB));
    ASSERT(handle(codes_handle_new_from_message(nullptr, const_cast<void*>(message), length)));
}


GribMemoryInput::~GribMemoryInput() = default;


bool GribMemoryInput::sameAs(const MIRInput& other) const {
    return this == &other;
}


void GribMemoryInput::print(std::ostream& out) const {
    out << "GribMemoryInput[]";
}


}  // namespace input
}  // namespace mir
