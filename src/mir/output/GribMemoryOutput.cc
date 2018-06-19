/*
 * (C) Copyright 1996- ECMWF.
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

#include "mir/output/GribMemoryOutput.h"


namespace mir {
namespace output {


GribMemoryOutput::GribMemoryOutput(void *message, size_t size):
    message_(message),
    size_(size),
    length_(0),
    saved_(0),
    interpolated_(0) {
}


GribMemoryOutput::~GribMemoryOutput() = default;


bool GribMemoryOutput::sameAs(const MIROutput& other) const {
    return this == &other;
}

void GribMemoryOutput::out(const void* message, size_t length, bool iterpolated) {
    ASSERT(length <= size_);
    length_ = length;
    ::memcpy(message_, message, length);

    if(iterpolated) {
        interpolated_++;
    } else {
        saved_ ++;
    }
}


void GribMemoryOutput::print(std::ostream& out) const {
    out << "GribMemoryOutput[]";
}


}  // namespace output
}  // namespace mir

