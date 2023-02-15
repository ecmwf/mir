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


#pragma once

#include "mir/util/Exceptions.h"


namespace mir::netcdf::exception {


using mir::exception::SeriousBug;
using mir::exception::UserError;


struct NCError : public eckit::Exception {
public:
    NCError(int e, const std::string& call, const std::string& path);
};


class MergeError : public eckit::Exception {
public:
    MergeError(const std::string& message) : eckit::Exception("MergeError: " + message) {}
};


inline int _nc_call(int e, const char* call, const std::string& path) {
    if (e != 0) {
        throw NCError(e, call, path);
    }
    return e;
}


#define NC_CALL(a, path) ::mir::netcdf::exception::_nc_call(a, #a, path)


}  // namespace mir::netcdf::exception
