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


#include "mir/netcdf/InputMatrix.h"

#include <ostream>

#include <netcdf.h>

#include "mir/netcdf/Codec.h"
#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/Mapper.h"
#include "mir/netcdf/NCFile.h"
#include "mir/netcdf/Type.h"
#include "mir/netcdf/Value.h"


namespace mir::netcdf {


InputMatrix::InputMatrix(Type& type, int varid, const std::string& name, size_t size, NCFile& file) :
    Matrix(type, name, size), file_(file), varid_(varid) {}

InputMatrix::~InputMatrix() = default;

void InputMatrix::print(std::ostream& out) const {
    out << "InputMatrix[name=" << name_ << ",type=" << *type_ << ", size=" << size_ << "]";
}


template <class V, class G>
static void _get(V& v, size_t size, int varid, NCFile& file, G get) {
    v.resize(size);
    int nc = file.open();
    NC_CALL(get(nc, varid, &v[0]), file.path());
    file.close();
}


void InputMatrix::read(std::vector<double>& v) const {
    _get(v, size_, varid_, file_, &nc_get_var_double);
    if (codec_ != nullptr) {
        codec_->decode(v);
    }
}


void InputMatrix::read(std::vector<float>& v) const {
    _get(v, size_, varid_, file_, &nc_get_var_float);
    if (codec_ != nullptr) {
        codec_->decode(v);
    }
}


void InputMatrix::read(std::vector<long>& v) const {
    _get(v, size_, varid_, file_, &nc_get_var_long);
    if (codec_ != nullptr) {
        codec_->decode(v);
    }
}


void InputMatrix::read(std::vector<short>& v) const {
    _get(v, size_, varid_, file_, &nc_get_var_short);
    if (codec_ != nullptr) {
        codec_->decode(v);
    }
}


void InputMatrix::read(std::vector<unsigned char>& v) const {
    _get(v, size_, varid_, file_, &nc_get_var_ubyte);
    if (codec_ != nullptr) {
        codec_->decode(v);
    }
}


void InputMatrix::read(std::vector<long long>& v) const {
    _get(v, size_, varid_, file_, &nc_get_var_longlong);
    if (codec_ != nullptr) {
        codec_->decode(v);
    }
}


template <class V, class G>
static void _get_slab(V& v, const std::vector<size_t>& start, const std::vector<size_t>& count, int varid, NCFile& file,
                      G get) {
    size_t size = std::accumulate(count.begin(), count.end(), size_t(1), std::multiplies<size_t>());

    v.resize(size);
    int nc = file.open();
    NC_CALL(get(nc, varid, start.data(), count.data(), v.data()), file.path());
    file.close();
}


void InputMatrix::read(std::vector<double>& values, const std::vector<size_t>& start,
                       const std::vector<size_t>& count) const {
    _get_slab(values, start, count, varid_, file_, &nc_get_vara_double);
    if (codec_ != nullptr) {
        codec_->decode(values);
    }
}


void InputMatrix::read(std::vector<float>& values, const std::vector<size_t>& start,
                       const std::vector<size_t>& count) const {
    _get_slab(values, start, count, varid_, file_, &nc_get_vara_float);
    if (codec_ != nullptr) {
        codec_->decode(values);
    }
}


void InputMatrix::read(std::vector<long>& values, const std::vector<size_t>& start,
                       const std::vector<size_t>& count) const {
    _get_slab(values, start, count, varid_, file_, &nc_get_vara_long);
    if (codec_ != nullptr) {
        codec_->decode(values);
    }
}


void InputMatrix::read(std::vector<short>& values, const std::vector<size_t>& start,
                       const std::vector<size_t>& count) const {
    _get_slab(values, start, count, varid_, file_, &nc_get_vara_short);
    if (codec_ != nullptr) {
        codec_->decode(values);
    }
}


void InputMatrix::read(std::vector<unsigned char>& values, const std::vector<size_t>& start,
                       const std::vector<size_t>& count) const {
    _get_slab(values, start, count, varid_, file_, &nc_get_vara_uchar);
    if (codec_ != nullptr) {
        codec_->decode(values);
    }
}


void InputMatrix::read(std::vector<long long>& values, const std::vector<size_t>& start,
                       const std::vector<size_t>& count) const {
    _get_slab(values, start, count, varid_, file_, &nc_get_vara_longlong);
    if (codec_ != nullptr) {
        codec_->decode(values);
    }
}


}  // namespace mir::netcdf
