// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
