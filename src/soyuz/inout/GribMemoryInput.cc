// File GribMemoryInput.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "GribMemoryInput.h"

#include "eckit/exception/Exceptions.h"

#include "soyuz/util/Grib.h"

#include <iostream>



GribMemoryInput::GribMemoryInput(const char* message, size_t length)
{
    ASSERT(handle(grib_handle_new_from_message(0, const_cast<char*>(message), length)));
}

GribMemoryInput::~GribMemoryInput() {
}

void GribMemoryInput::print(std::ostream& out) const {
    out << "GribMemoryInput[]";
}
