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


#include "mir/output/GribFileOutput.h"
#include "eckit/io/DataHandle.h"


namespace mir {
namespace output {


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


}  // namespace output
}  // namespace mir
