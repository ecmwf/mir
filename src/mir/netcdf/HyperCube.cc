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


#include "mir/netcdf/HyperCube.h"
#include "mir/netcdf/Exceptions.h"

namespace mir {
namespace netcdf {


static void addLoop2(int d, size_t which, size_t where, size_t count, size_t depth, const HyperCube::Dimensions& dims,
                     HyperCube::Remapping& remap, const HyperCube::Dimensions& mul, size_t sum) {
    size_t k = 0;
    auto du  = size_t(d);

    size_t muld  = mul[du];
    size_t dimsd = dims[du];

    for (size_t i = 0; i < dimsd; i++, k++) {
        if (which == du && i == where) {
            k += count;
        }

        size_t s = sum + k * muld;
        if (du == depth) {
            remap.push_back(s);
        }
        else {
            addLoop2(d + 1, which, where, count, depth, dims, remap, mul, s);
        }
    }
}


HyperCube HyperCube::addToDimension(size_t which, size_t where, size_t howMuch, Remapping& remap) const {

    remap.clear();
    remap.reserve(count());

    Dimensions newdims = dimensions_;
    // Coordinates coord(dimensions_.size());


    newdims[which] += howMuch;

    HyperCube target(newdims);

    Dimensions mul(dimensions_.size());
    size_t n = 1;
    for (int i = int(mul.size()) - 1; i >= 0; i--) {
        auto iu = size_t(i);
        mul[iu] = n;
        n *= newdims[iu];
    }
    addLoop2(0, which, where, howMuch, dimensions_.size() - 1, dimensions_, remap, mul, 0);

    return target;
}


std::ostream& operator<<(std::ostream& out, const HyperCube& cube) {
    out << "HyperCube[size=" << cube.count() << ",coordinates=";
    char sep = '(';
    for (const auto& j : cube.dimensions()) {
        out << sep << j;
        sep = ',';
    }
    out << ')';
    out << ']';
    return out;
}


void HyperCube::coordinates(size_t index, Coordinates& result) const {
    ASSERT(result.size() == dimensions_.size());

    for (int i = int(dimensions_.size()) - 1; i >= 0; i--) {
        auto iu    = size_t(i);
        result[iu] = (index % dimensions_[iu]);
        index /= dimensions_[iu];
    }

    ASSERT(index == 0);
}


}  // namespace netcdf
}  // namespace mir
