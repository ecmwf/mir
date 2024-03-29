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


#include "mir/netcdf/NCFile.h"
#include "mir/netcdf/Exceptions.h"

#include <netcdf.h>

namespace mir::netcdf {

NCFile::NCFile(const std::string& path) : path_(path), nc_(-1), open_(false) {}

NCFile::~NCFile() {
    ASSERT(!open_);
    if (nc_ != -1) {
        NC_CALL(nc_close(nc_), path_);
    }
}

void NCFile::print(std::ostream& out) const {
    out << "NCFile[path=" << path_ << "]";
}

int NCFile::open() {
    ASSERT(!open_);
    open_ = true;
    if (nc_ == -1) {
        NC_CALL(nc_open(path_.c_str(), NC_NOWRITE, &nc_), path_);
    }
    return nc_;
}

void NCFile::close() {
    ASSERT(open_);
    open_ = false;
}

const std::string& NCFile::path() const {
    return path_;
}

}  // namespace mir::netcdf
