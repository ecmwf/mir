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


#include "mir/input/GribFileInput.h"

#include "eckit/io/BufferedHandle.h"


namespace mir {
namespace input {


GribFileInput::GribFileInput(const eckit::PathName& path, size_t skip, size_t step) :
    GribStreamInput(skip, step), path_(path), handle_(nullptr) {}

GribFileInput::GribFileInput(const eckit::PathName& path, off_t offset) :
    GribStreamInput(offset), path_(path), handle_(nullptr) {}

GribFileInput::GribFileInput(const eckit::PathName& path) : path_(path), handle_(nullptr) {}

GribFileInput::~GribFileInput() {
    if (handle_ != nullptr) {
        handle_->close();
        delete handle_;
    }
}

bool GribFileInput::sameAs(const MIRInput& other) const {
    auto o = dynamic_cast<const GribFileInput*>(&other);
    return (o != nullptr) && (skip_ == o->skip_) && (step_ == o->step_) && (path_ == o->path_);
}

void GribFileInput::print(std::ostream& out) const {
    out << "GribFileInput[path=" << path_ << ",skip=" << skip_ << ", step=" << step_ << "]";
}

eckit::DataHandle& GribFileInput::dataHandle() {
    if (handle_ == nullptr) {
        handle_ = new eckit::BufferedHandle(path_.fileHandle());
        handle_->openForRead();
    }
    return *handle_;
}

static MIRInputBuilder<GribFileInput> input(0x47524942);  // "GRIB"


}  // namespace input
}  // namespace mir
