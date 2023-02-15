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
