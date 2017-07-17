/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @author Florian Rathgeber
/// @date   Jul 2017


#include "mir/input/GribMemoryStreamInput.h"

#include "eckit/io/BufferedHandle.h"
#include "eckit/io/MemoryHandle.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace input {

GribMemoryStreamInput::GribMemoryStreamInput(const void* message, size_t length, size_t skip, size_t step):
    GribStreamInput(skip, step),
    message_(message),
    length_(length),
    handle_(0) {
}

GribMemoryStreamInput::GribMemoryStreamInput(const void* message, size_t length, off_t offset):
    GribStreamInput(offset),
    message_(message),
    length_(length),
    handle_(0) {
}

GribMemoryStreamInput::GribMemoryStreamInput(const void* message, size_t length):
    GribStreamInput(),
    message_(message),
    length_(length),
    handle_(0) {
}

GribMemoryStreamInput::~GribMemoryStreamInput() {
    if (handle_) {
        handle_->close();
        delete handle_;
    }
}

size_t GribMemoryStreamInput::dimensions() const {
    // FIXME
    eckit::Log::warning() << "GribMemoryStreamInput::dimensions() returning 1 (hardcoded!)" << std::endl;
    return 1;
}

bool GribMemoryStreamInput::sameAs(const MIRInput& other) const {
    return this == &other;
}

void GribMemoryStreamInput::print(std::ostream &out) const {
    out << "GribMemoryStreamInput[handle=" << handle_ << ",skip=" << skip_ << ", step=" << step_ << "]";
}

eckit::DataHandle &GribMemoryStreamInput::dataHandle() {
    if (!handle_) {
        handle_ = new eckit::BufferedHandle(new eckit::MemoryHandle(message_, length_));
        handle_->openForRead();
    }
    return *handle_;
}

}  // namespace input
}  // namespace mir
