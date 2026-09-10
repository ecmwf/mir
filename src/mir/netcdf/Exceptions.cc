// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/Exceptions.h"

#include <netcdf.h>


namespace mir::netcdf::exception {


NCError::NCError(int e, const std::string& call, const std::string& path) :
    eckit::Exception(std::string("NCError: ") + nc_strerror(e) + +" " + call + " (" + path + ")") {}


}  // namespace mir::netcdf::exception
