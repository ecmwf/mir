/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date   May 2016


#include "mir/method/MIRGrid.h"

#include "eckit/geometry/Point3.h"
#include "atlas/array.h"
#include "atlas/mesh/Mesh.h"
#include "mir/method/MethodWeighted.h"
#include "eckit/utils/MD5.h"


namespace mir {
namespace method {


MIRGrid::MIRGrid(const atlas::Grid& grid, const util::Domain& domain) :
    domain_(domain),
    grid_(grid),
    mesh_(0),
    coordsLonLat_(0) {
}



MIRGrid::MIRGrid(const MIRGrid& other) :
    domain_(other.domain_),
    grid_(other.grid_),
    mesh_(0),
    coordsLonLat_(0) {
}

const util::Domain& MIRGrid::domain() const {
    return domain_;
}


MIRGrid::operator const atlas::Grid&() const {
    return grid_;
}


atlas::Mesh& MIRGrid::mesh(const MethodWeighted& method) const {
    if (mesh_ == 0) {
        mesh_ = &(method.generateMeshAndCache(grid_));
    }
    return *mesh_;
}


const atlas::array::Array& MIRGrid::coordsLonLat() const {
    using namespace atlas::array;

    if (!coordsLonLat_) {
        coordsLonLat_.reset(Array::create< double >(grid_.size(), 2));
        ArrayView< double, 2 > lonlat = make_view< double, 2 >(*coordsLonLat_);

        size_t i = 0;
        for (atlas::PointLonLat p : grid_.lonlat()) {
            lonlat(i, 0) = p.lon();
            lonlat(i, 1) = p.lat();
            ++i;
        }
    }

    return *coordsLonLat_;
}


const atlas::array::Array& MIRGrid::coordsXYZ() const {
    using namespace atlas::array;

    if (!coordsXYZ_) {
        coordsXYZ_.reset(Array::create< double >(grid_.size(), 3));
        ArrayView< double, 2 > xyz = make_view< double, 2 >(*coordsXYZ_);

        size_t i = 0;
        for (atlas::PointLonLat p : grid_.lonlat()) {
            atlas::PointXYZ x = atlas::lonlat_to_geocentric(p);
            xyz(i, 0) = x.x();
            xyz(i, 1) = x.y();
            xyz(i, 2) = x.z();
            ++i;
        }
    }

    return *coordsXYZ_;
}

void MIRGrid::hash(eckit::MD5 &md5) const {
    md5 << grid_ << domain_;
}

std::string MIRGrid::name() const {
    return grid_.name();
}


std::string MIRGrid::uid() const {
    return grid_.uid();
}

size_t MIRGrid::size() const {
    return grid_.size();
}


}  // namespace method
}  // namespace mir
