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
