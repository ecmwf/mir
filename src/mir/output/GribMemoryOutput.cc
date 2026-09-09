// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cstring>
#include <ostream>

#include "mir/output/GribMemoryOutput.h"
#include "mir/util/Exceptions.h"


namespace mir::output {


GribMemoryOutput::GribMemoryOutput(void* message, size_t size) : message_(message), size_(size), length_(0) {}


GribMemoryOutput::~GribMemoryOutput() = default;


bool GribMemoryOutput::sameAs(const MIROutput& other) const {
    return this == &other;
}

void GribMemoryOutput::out(const void* message, size_t length, bool /*interpolated*/) {
    ASSERT(length <= size_);
    length_ = length;
    std::memcpy(message_, message, length);
}


void GribMemoryOutput::print(std::ostream& out) const {
    out << "GribMemoryOutput[]";
}


}  // namespace mir::output
