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


#pragma once

#include "mir/api/mir_config.h"

#if mir_HAVE_ATLAS
#include "atlas/functionspace.h"
#include "atlas/grid.h"
#include "atlas/interpolation.h"
#include "atlas/interpolation/element/Quad3D.h"
#include "atlas/interpolation/element/Triag3D.h"
#include "atlas/interpolation/method/PointIndex3.h"
#include "atlas/interpolation/method/Ray.h"
#include "atlas/library/Library.h"
#include "atlas/library/config.h"
#include "atlas/mesh.h"
#include "atlas/mesh/actions/BuildCellCentres.h"
#include "atlas/mesh/actions/BuildNode2CellConnectivity.h"
#include "atlas/mesh/actions/BuildXYZField.h"
#include "atlas/meshgenerator.h"
#include "atlas/numerics/Nabla.h"
#include "atlas/numerics/fvm/Method.h"
#include "atlas/option.h"
#include "atlas/output/Gmsh.h"
#include "atlas/trans/LegendreCacheCreator.h"
#include "atlas/trans/Trans.h"
#include "atlas/util/Config.h"
#include "atlas/util/GaussianLatitudes.h"
#include "atlas/util/Point.h"
#else
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"

#include "mir/util/BoundingBox.h"
#include "mir/util/Projection.h"
#include "mir/util/Types.h"


namespace eckit::option {
class CmdArgs;
}  // namespace eckit::option


namespace atlas {


using PointXY     = eckit::geo::Point2;
using PointXYZ    = eckit::geo::Point3;
using PointLonLat = eckit::geo::PointLonLat;


struct Domain {
    struct Range {
        Range(double min, double max);
        const double min_;
        const double max_;
    };
    const Range lon_;
    const Range lat_;

    explicit Domain(Range&& lon = {mir::Longitude::GREENWICH.value(), mir::Longitude::GLOBE.value()},
                    Range&& lat = {mir::Latitude::SOUTH_POLE.value(), mir::Latitude::NORTH_POLE.value()},
                    const std::string& /*units*/ = "");

    explicit Domain(const mir::util::BoundingBox& bbox) :
        Domain{{bbox.west().value(), bbox.east().value()}, {bbox.south().value(), bbox.north().value()}} {}

    inline double north() const { return lat_.max_; }
    inline double south() const { return lat_.min_; }
    inline double west() const { return lon_.min_; }
    inline double east() const { return lon_.max_; }
    bool zonal_band() const;
    bool global() const;
    bool operator==(const Domain&) const;
    explicit operator bool() const { return true; }
};


struct Library {
    static Library& instance() {
        static Library me;
        return me;
    }
    void initialise(const eckit::option::CmdArgs&) {}
    void finalise() {}
};


namespace util {


struct Config : public eckit::LocalConfiguration {
    Config() : LocalConfiguration('.') {}
    Config(const std::string&, const std::string&) : LocalConfiguration('.') {}
    template <typename T>
    Config& set(const std::string& a, const T& b) {
        LocalConfiguration::set(a, b);
        return *this;
    }
};


}  // namespace util


namespace grid {


struct Spacing : protected std::vector<double> {
    using Spec = util::Config;
    Spec spec_;
    Spec spec() const { return spec_; }

    using vector = vector<value_type>;
    using vector::vector;

    using vector::back;
    using vector::front;
    using vector::size;
    using vector::operator[];

    using vector::begin;
    using vector::end;

    value_type min() const { return front() < back() ? front() : back(); }
    value_type max() const { return front() > back() ? front() : back(); }
};


struct LinearSpacing : public Spacing {
    LinearSpacing(value_type a, value_type b, long n, bool endpoint = true);
    LinearSpacing() = default;
    value_type step() const { return size() > 1 ? operator[](1) - operator[](0) : 0; }
};


}  // namespace grid


struct MeshGenerator {
    using Parameters = util::Config;
};


using idx_t = long;


class Grid {
public:
    using Spec = util::Config;
    Spec spec_;
    Spec spec() const { return spec_; }

    using Projection = mir::util::Projection;

    explicit operator bool() const { return true; }
    Projection projection() const;
};


struct StructuredGrid : Grid {
    using XSpace = grid::Spacing;
    using YSpace = grid::Spacing;
    using Grid::Grid;
    StructuredGrid(const Grid&);
    StructuredGrid(const XSpace& lon, const YSpace& lat, const Projection& = Projection(), const Domain& = Domain()) :
        lon_(lon), lat_(lat) {}
    idx_t nx(idx_t j) const { return pl_.at(static_cast<size_t>(j)); }
    idx_t nx() const;
    idx_t ny() const { return static_cast<idx_t>(pl_.size()); }

protected:
    std::vector<long> pl_;
    XSpace lon_;
    YSpace lat_;
};


using RegularGrid = StructuredGrid;


namespace trans {
class LegendreCache {
public:
    LegendreCache(const void*, size_t);
};
}  // namespace trans


}  // namespace atlas


#endif
