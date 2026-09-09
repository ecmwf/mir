// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
