// File GribMemoryOutput.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/output/GribMemoryOutput.h"

#include "eckit/exception/Exceptions.h"

#include <iostream>

GribMemoryOutput::GribMemoryOutput(void *message, size_t size):
    message_(message),
    size_(size),
    length_(0),
    interpolated_(0),
    saved_(0)
{
}

GribMemoryOutput::~GribMemoryOutput() {
}

void GribMemoryOutput::out(const void* message, size_t length, bool iterpolated) {
    ASSERT(length < size_);
    length_ = length;
    ::memcpy(message_, message, length);

    if(iterpolated) {
        interpolated_++;
    }
    else {
        saved_ ++;
    }
}


void GribMemoryOutput::print(std::ostream& out) const {
    out << "GribMemoryOutput[]";
}
