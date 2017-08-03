/*
 * (C) Copyright 1996-2015 ECMWF.
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


#include "mir/input/GribFileInput.h"

#include "eckit/io/BufferedHandle.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace input {


GribFileInput::GribFileInput(const eckit::PathName &path, size_t skip, size_t step):
    GribStreamInput(skip, step),
    path_(path), handle_(0) {
}

GribFileInput::GribFileInput(const eckit::PathName &path, off_t offset):
    GribStreamInput(offset),
    path_(path), handle_(0) {
}

GribFileInput::GribFileInput(const eckit::PathName &path):
    GribStreamInput(),
    path_(path), handle_(0) {
}

GribFileInput::~GribFileInput() {
    if (handle_) {
        handle_->close();
        delete handle_;
    }
}

size_t GribFileInput::dimensions() const {
    // FIXME
    eckit::Log::warning() << "GribFileInput::dimensions() returning 1 (hardcoded!)" << std::endl;
    return 1;
}

bool GribFileInput::sameAs(const MIRInput& other) const {
    const GribFileInput* o = dynamic_cast<const GribFileInput*>(&other);
    return o && (skip_ == o->skip_) && (step_ == o->step_) && (path_ == o->path_);
}

void GribFileInput::print(std::ostream &out) const {
    out << "GribFileInput[path=" << path_ << ",skip=" << skip_ << ", step=" << step_ << "]";
}

eckit::DataHandle &GribFileInput::dataHandle() {
    if (!handle_) {
        handle_ = new eckit::BufferedHandle(path_.fileHandle());
        handle_->openForRead();
    }
    return *handle_;
}

static MIRInputBuilder<GribFileInput> input(0x47524942); // "GRIB"


}  // namespace input
}  // namespace mir

