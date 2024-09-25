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

#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/geometry/Point2.h"
#include "eckit/geometry/Point3.h"
#include "eckit/geometry/SphereT.h"
#include "eckit/utils/Hash.h"

#include "mir/api/mir_config.h"
#include "mir/util/Types.h"

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
#include "atlas/projection/detail/ProjectionFactory.h"
#include "atlas/trans/LegendreCacheCreator.h"
#include "atlas/trans/Trans.h"
#include "atlas/util/Config.h"
#include "atlas/util/Earth.h"
#include "atlas/util/GaussianLatitudes.h"
#include "atlas/util/Point.h"
#include "atlas/util/Rotation.h"
#else


namespace eckit {
namespace option {
class CmdArgs;
}
}  // namespace eckit


namespace atlas {


using PointXY  = eckit::geometry::Point2;
using PointXYZ = eckit::geometry::Point3;


struct PointLonLat : public eckit::geometry::Point2 {
    using Point2::Point2;
    double lon() const { return x_[0]; }
    double lat() const { return x_[1]; }
    double& lon() { return x_[0]; }
    double& lat() { return x_[1]; }
    //    operator mir::PointLatLon() const { return {lat(), lon()}; }
};


struct Domain {
    struct Range {
        Range(double min, double max);
        const double min_;
        const double max_;
    };
    const Range lon_ = {mir::Longitude::GREENWICH.value(), mir::Longitude::GLOBE.value()};
    const Range lat_ = {mir::Latitude::SOUTH_POLE.value(), mir::Latitude::NORTH_POLE.value()};
    Domain()         = default;
    Domain(Range&& lon, Range&& lat, const std::string& /*units*/ = "");
    inline double north() const { return lat_.max_; }
    inline double south() const { return lat_.min_; }
    inline double west() const { return lon_.min_; }
    inline double east() const { return lon_.max_; }
    bool zonal_band() const;
    bool global() const;
    bool operator==(const Domain&) const;
    explicit operator bool() const { return true; }
};


using RectangularDomain = Domain;


struct Library {
    static Library& instance() {
        static Library me;
        return me;
    }
    void initialise(const eckit::option::CmdArgs&) {}
    void finalise() {}
};


namespace util {


void gaussian_latitudes_npole_equator(size_t N, double* latitudes);
void gaussian_latitudes_npole_spole(size_t N, double* latitudes);
void gaussian_quadrature_npole_spole(size_t N, double* latitudes, double* weights);


struct DatumIFS {
    static constexpr double radius() { return 6371229.; }
};


using Earth = eckit::geometry::SphereT<DatumIFS>;


struct Rotation : PointLonLat {
    // no rotation supported
    Rotation(const PointLonLat& southPole = {mir::Longitude::GREENWICH.value(), mir::Latitude::SOUTH_POLE.value()});
    bool rotated() const { return false; }
    inline void rotate(const double*) const {}
    inline mir::Longitude south_pole_longitude() const { return lon(); }
    inline mir::Latitude south_pole_latitude() const { return lat(); }
};


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


namespace projection {
struct ProjectionFactory {
    static bool has(const std::string&);
};
}  // namespace projection


struct MeshGenerator {
    using Parameters = util::Config;
};


class Projection {
public:
    using Spec = util::Config;
    Spec spec_;
    Spec spec() const { return spec_; }
    void hash(eckit::Hash& h) const { spec_.hash(h); }

    // no projection supported
    Projection() = default;
    Projection(const Spec& spec) : spec_(spec) {}
    explicit operator bool() const { return true; }
    mir::Point2 xy(const mir::Point2& p) const { return p; }
    mir::Point2 lonlat(const mir::Point2& p) const { return p; }
    Domain lonlatBoundingBox(const Domain& r) const { return r; }
};


using idx_t = long;


class Grid {
public:
    using Spec = util::Config;
    Spec spec_;
    Spec spec() const { return spec_; }
    void hash(eckit::Hash& h) const { spec_.hash(h); }

    using Projection = atlas::Projection;

    Grid() = default;
    Grid(const Spec& spec) : spec_(spec) {}
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


struct GaussianGrid : StructuredGrid {
    GaussianGrid(const std::string& name, const Domain& = Domain());
    GaussianGrid(const std::vector<long>& pl, const Domain& = Domain());
    const std::vector<long>& nx() const { return pl_; }
};


struct UnstructuredGrid : Grid {
    std::vector<PointXY> points_;
    UnstructuredGrid(std::vector<PointXY>&&);
};


using RegularGrid         = StructuredGrid;
using RegularGaussianGrid = GaussianGrid;
using ReducedGaussianGrid = GaussianGrid;


namespace trans {
class LegendreCache {
public:
    LegendreCache(const void*, size_t);
};
}  // namespace trans


}  // namespace atlas


#endif


namespace mir::util {


using atlas::util::Earth;


}  // namespace mir::util
