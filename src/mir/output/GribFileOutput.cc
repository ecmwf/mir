// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/output/GribFileOutput.h"
#include "eckit/io/DataHandle.h"


namespace mir::output {


GribFileOutput::GribFileOutput(const eckit::PathName& path, bool append) :
    path_(path), handle_(nullptr), append_(append) {}


GribFileOutput::~GribFileOutput() {
    if (handle_ != nullptr) {
        handle_->close();
        delete handle_;
    }
}


bool GribFileOutput::sameAs(const MIROutput& other) const {
    const auto* o = dynamic_cast<const GribFileOutput*>(&other);
    return (o != nullptr) && (path_ == o->path_) && (append_ == o->append_);
}


void GribFileOutput::print(std::ostream& out) const {
    out << "GribFileOutput[path=" << path_ << "]";
}


eckit::DataHandle& GribFileOutput::dataHandle() {
    if (handle_ == nullptr) {
        handle_ = path_.fileHandle();
        if (append_) {
            handle_->openForAppend(0);
        }
        else {
            handle_->openForWrite(0);
        }
    }
    return *handle_;
}


static const MIROutputBuilder<GribFileOutput> output("grib");


}  // namespace mir::output
