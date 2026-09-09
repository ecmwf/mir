// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/Value.h"
#include "mir/netcdf/ValueT.h"

#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/Type.h"

#include <netcdf.h>

namespace mir::netcdf {

Value::Value(Type& type) : type_(type) {}

Value::~Value() = default;

Value* Value::newFromString(const std::string& s) {
    return new ValueT<std::string>(Type::lookup(NC_CHAR), s);
}

}  // namespace mir::netcdf
