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


#include "mir/netcdf/Matrix.h"

#include <ostream>
#include <sstream>

#include "mir/netcdf/Codec.h"
#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/Mapper.h"
#include "mir/netcdf/MergeCoordinateMatrix.h"
#include "mir/netcdf/MergeDataMatrix.h"
#include "mir/netcdf/Reshape.h"
#include "mir/netcdf/Type.h"
#include "mir/netcdf/Value.h"


namespace mir {
namespace netcdf {


Matrix::Matrix(Type& type, const std::string& name, size_t size) :
    type_(&type), missingValue_(nullptr), codec_(nullptr), name_(name), size_(size) {}


Matrix::~Matrix() {
    delete missingValue_;
    if (codec_ != nullptr) {
        codec_->detach();
    }
}


Type& Matrix::type() const {
    return *type_;
}


void Matrix::type(Type& type) {
    type_ = &type;
}


const std::string& Matrix::name() const {
    return name_;
}


Value* Matrix::missingValue() const {
    return missingValue_;
}


void Matrix::dump(std::ostream& out) const {
    type_->dump(out, *this);
}


void Matrix::dumpTree(std::ostream& out, size_t depth) const {
    out << std::string(3 * depth, ' ') << *this << std::endl;
}


void Matrix::save(int nc, int varid, const std::string& path) const {
    type_->save(*this, nc, varid, path);
}


void Matrix::printValues(std::ostream& out) const {
    type_->printValues(out, *this);
}


void Matrix::missingValue(Value* value) {
    delete missingValue_;
    missingValue_ = value;
}


size_t Matrix::size() const {
    return size_;
}


void Matrix::codec(Codec* codec) {

    ASSERT(!codec_);
    if (codec != nullptr) {
        codec->attach();
    }
    if (codec_ != nullptr) {
        codec_->detach();
    }
    codec_ = codec;
}


Codec* Matrix::codec() const {
    return codec_;
}


void Matrix::read(std::vector<double>&) const {
    std::ostringstream os;
    os << "Matrix::read(std::vector<double>) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Matrix::read(std::vector<float>&) const {
    std::ostringstream os;
    os << "Matrix::read(std::vector<float>) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Matrix::read(std::vector<long>&) const {
    std::ostringstream os;
    os << "Matrix::read(std::vector<long>) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Matrix::read(std::vector<short>&) const {
    std::ostringstream os;
    os << "Matrix::read(std::vector<short>) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Matrix::read(std::vector<unsigned char>&) const {
    std::ostringstream os;
    os << "Matrix::read(std::vector<unsigned char>) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Matrix::read(std::vector<long long>&) const {
    std::ostringstream os;
    os << "Matrix::read(std::vector<long long>) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Matrix::read(std::vector<std::string>&) const {
    std::ostringstream os;
    os << "Matrix::read(std::vector<std::string>) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Matrix::read(std::vector<double>&, const std::vector<size_t>& /*start*/,
                  const std::vector<size_t>& /*count*/) const {
    std::ostringstream os;
    os << "Matrix::read() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Matrix::read(std::vector<float>&, const std::vector<size_t>& /*start*/,
                  const std::vector<size_t>& /*count*/) const {
    std::ostringstream os;
    os << "Matrix::read() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Matrix::read(std::vector<long>&, const std::vector<size_t>& /*start*/,
                  const std::vector<size_t>& /*count*/) const {
    std::ostringstream os;
    os << "Matrix::read() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Matrix::read(std::vector<short>&, const std::vector<size_t>& /*start*/,
                  const std::vector<size_t>& /*count*/) const {
    std::ostringstream os;
    os << "Matrix::read() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Matrix::read(std::vector<unsigned char>&, const std::vector<size_t>& /*start*/,
                  const std::vector<size_t>& /*count*/) const {
    std::ostringstream os;
    os << "Matrix::read() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Matrix::read(std::vector<long long>&, const std::vector<size_t>& /*start*/,
                  const std::vector<size_t>& /*count*/) const {
    std::ostringstream os;
    os << "Matrix::read() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


}  // namespace netcdf
}  // namespace mir
