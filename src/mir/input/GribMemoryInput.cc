// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/input/GribMemoryInput.h"

#include <ostream>

#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::input {


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


}  // namespace mir::input
