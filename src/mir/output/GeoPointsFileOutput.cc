// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/output/GeoPointsFileOutput.h"

#include "eckit/filesystem/PathName.h"
#include "eckit/io/DataHandle.h"


namespace mir::output {


GeoPointsFileOutput::GeoPointsFileOutput(const std::string& path, bool binary) : path_(path), binary_(binary) {}


GeoPointsFileOutput::~GeoPointsFileOutput() = default;


eckit::DataHandle& GeoPointsFileOutput::dataHandle() const {
    if (!handle_) {
        eckit::PathName path(path_);
        handle_.reset(path.fileHandle());
        handle_->openForWrite(0);
    }
    return *handle_;
}


bool GeoPointsFileOutput::sameAs(const MIROutput& other) const {
    const auto* o = dynamic_cast<const GeoPointsFileOutput*>(&other);
    return (o != nullptr) && (path_ == o->path_) && (binary_ == o->binary_);
}


void GeoPointsFileOutput::print(std::ostream& out) const {
    out << "GeoPointsFileOutput[path=" << path_ << "]";
}


}  // namespace mir::output
