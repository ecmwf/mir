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


#include "mir/output/GribFileOutput.h"
#include "eckit/io/DataHandle.h"


namespace mir {
namespace output {


GribFileOutput::GribFileOutput(const eckit::PathName &path):
    path_(path), handle_(0) {
}


GribFileOutput::~GribFileOutput() {
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


}  // namespace output
}  // namespace mir

