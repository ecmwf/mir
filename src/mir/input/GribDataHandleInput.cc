// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
