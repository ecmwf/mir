// File GribFileInput.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "GribFileInput.h"
#include "eckit/io/DataHandle.h"
#include "eckit/io/BufferedHandle.h"

GribFileInput::GribFileInput(const eckit::PathName &path):
    path_(path), handle_(0) {
}

GribFileInput::~GribFileInput() {
    if (handle_) {
        handle_->close();
        delete handle_;
    }
}

void GribFileInput::print(std::ostream &out) const {
    out << "GribFileInput[path=" << path_ << "]";
}

eckit::DataHandle &GribFileInput::dataHandle() {
    if (!handle_) {
        handle_ = new eckit::BufferedHandle(path_.fileHandle());
        handle_->openForRead();
    }
    return *handle_;
}
