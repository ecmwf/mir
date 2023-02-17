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


#include <netcdf.h>

#include "mir/netcdf/ValueT.h"

#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/Type.h"

namespace mir::netcdf {

template <>
void ValueT<unsigned char>::createAttribute(int nc, int varid, const std::string& name, const std::string& path) {
    NC_CALL(nc_put_att_ubyte(nc, varid, name.c_str(), type_.code(), 1, &value_), path);
}

template <>
void ValueT<unsigned char>::dump(std::ostream& out) const {
    out << value_;
}


template <>
void ValueT<long long>::createAttribute(int nc, int varid, const std::string& name, const std::string& path) {
    NC_CALL(nc_put_att_longlong(nc, varid, name.c_str(), type_.code(), 1, &value_), path);
}

template <>
void ValueT<long long>::dump(std::ostream& out) const {
    out << value_;
}

template <>
void ValueT<short>::createAttribute(int nc, int varid, const std::string& name, const std::string& path) {
    NC_CALL(nc_put_att_short(nc, varid, name.c_str(), type_.code(), 1, &value_), path);
}

template <>
void ValueT<short>::dump(std::ostream& out) const {
    out << value_;
}

template <>
void ValueT<long>::createAttribute(int nc, int varid, const std::string& name, const std::string& path) {
    NC_CALL(nc_put_att_long(nc, varid, name.c_str(), type_.code(), 1, &value_), path);
}

template <>
void ValueT<long>::dump(std::ostream& out) const {
    out << value_;
}

template <>
void ValueT<std::string>::dump(std::ostream& out) const {
    out << '"' << value_ << '"';
}

template <>
void ValueT<std::string>::createAttribute(int nc, int varid, const std::string& name, const std::string& path) {
    NC_CALL(nc_put_att_text(nc, varid, name.c_str(), value_.size(), value_.c_str()), path);
}

template <>
void ValueT<float>::createAttribute(int nc, int varid, const std::string& name, const std::string& path) {
    NC_CALL(nc_put_att_float(nc, varid, name.c_str(), type_.code(), 1, &value_), path);
}


template <>
void ValueT<float>::dump(std::ostream& out) const {
    out << value_;
}

template <>
void ValueT<double>::createAttribute(int nc, int varid, const std::string& name, const std::string& path) {
    NC_CALL(nc_put_att_double(nc, varid, name.c_str(), type_.code(), 1, &value_), path);
}


template <>
void ValueT<double>::dump(std::ostream& out) const {
    out << value_;
}

}  // namespace mir::netcdf
