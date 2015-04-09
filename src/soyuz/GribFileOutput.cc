// File GribFileOutput.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "GribFileOutput.h"
#include "eckit/io/DataHandle.h"


GribFileOutput::GribFileOutput(const eckit::PathName &path):
    path_(path), handle_(0)
{
}

GribFileOutput::~GribFileOutput()
{
     if(handle_) {
        handle_->close();
        delete handle_;
    }
}

void GribFileOutput::print(std::ostream &out) const {
    out << "GribFileOutput[path=" << path_ << "]";
}

eckit::DataHandle& GribFileOutput::dataHandle() {
    if(!handle_) {
        handle_ = path_.fileHandle();
        handle_->openForWrite(0);
    }
    return *handle_;
}
