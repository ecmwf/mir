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


#include "mir/netcdf/Reshape.h"

#include "mir/netcdf/Exceptions.h"

namespace mir::netcdf {

Reshape::Reshape(const HyperCube& cube, size_t which, size_t where, size_t count, char tag) :
    cube_(cube.dimensions()),
    mul_(cube_.size()),
    which_(which),
    where_(where),
    count_(count),
    size_(int(cube_.size()) - 1),
    tag_(tag) {
    ASSERT(count);

    HyperCube::Dimensions newdims(cube_);
    newdims[which_] += count_;

    size_t n = 1;
    for (int i = size_; i >= 0; i--) {
        auto iu  = size_t(i);
        mul_[iu] = n;
        n *= newdims[iu];
    }

    ASSERT(which_ <= size_t(size_));
}

bool Reshape::merge(const Reshape& other) {

    if (which_ == other.which_) {
        ASSERT(other.where_ != where_);
        if (where_ + count_ == other.where_) {

            count_ += other.count_;
            HyperCube::Dimensions newdims(cube_);
            newdims[which_] += count_;

            size_t n = 1;
            for (int i = size_; i >= 0; i--) {
                auto iu  = size_t(i);
                mul_[iu] = n;
                n *= newdims[iu];
            }
            return true;
        }
    }
    return false;
}

Reshape::~Reshape() = default;

void Reshape::print(std::ostream& out) const {
    out << "Reshape[which=" << which_ << ",where=" << where_ << ",count=" << count_;

    out << ",";
    char sep = '{';
    for (const auto& j : cube_) {
        out << sep << j;
        sep = ',';
    }
    out << "}] " << this << " " << tag_;
}


size_t Reshape::operator()(size_t idx) const {
    size_t a = 0;

    for (int d = size_; d >= 0; d--) {
        auto du  = size_t(d);
        size_t c = cube_[du];
        size_t b = (idx % c);
        idx /= c;
        if (du == which_ && b >= where_) {
            b += count_;
        }
        a += b * mul_[du];
    }

    return a;
}

}  // namespace mir::netcdf
