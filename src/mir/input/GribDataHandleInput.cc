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


#include "mir/input/GribDataHandleInput.h"

#include "eckit/io/DataHandle.h"


namespace mir::input {

GribDataHandleInput::GribDataHandleInput(eckit::DataHandle& handle, size_t skip, size_t step) :
    GribStreamInput(skip, step), handle_(handle) {
    handle_.openForRead();
}

GribDataHandleInput::GribDataHandleInput(eckit::DataHandle& handle, off_t offset) :
    GribStreamInput(offset), handle_(handle) {
    handle_.openForRead();
}

GribDataHandleInput::GribDataHandleInput(eckit::DataHandle& handle) : handle_(handle) {
    handle_.openForRead();
}

GribDataHandleInput::~GribDataHandleInput() {
    handle_.close();
}

bool GribDataHandleInput::sameAs(const MIRInput& other) const {
    return this == &other;
}

void GribDataHandleInput::print(std::ostream& out) const {
    out << "GribDataHandleInput[handle=" << handle_ << ",skip=" << skip_ << ", step=" << step_ << "]";
}

eckit::DataHandle& GribDataHandleInput::dataHandle() {
    return handle_;
}

}  // namespace mir::input
