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


#ifndef mir_util_Atlas_h
#define mir_util_Atlas_h

#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/geometry/Point2.h"
#include "eckit/geometry/Point3.h"
#include "eckit/geometry/SphereT.h"

#include "mir/util/Types.h"


#if defined(mir_HAVE_ATLAS)
#error "mir/util/Atlas.h cannot be included with Atlas present"
#endif


namespace eckit {
namespace option {
struct CmdArgs;
}
}  // namespace eckit


namespace atlas {


using PointXY  = eckit::geometry::Point2;
using PointXYZ = eckit::geometry::Point3;


struct PointLonLat : public eckit::geometry::Point2 {
    using Point2::Point2;
    double lon() const { return x_[0]; }
    double lat() const { return x_[1]; }
    //    operator mir::PointLatLon() const { return {lat(), lon()}; }
};


struct Domain {
    struct Range {
        Range(double min, double max);
        const double min_;
        const double max_;
    };
    const Range lon_;
    const Range lat_;
    Domain(Range&& lon, Range&& lat, std::string /*units*/ = "");
    double north() const { return lat_.max_; }
    double south() const { return lat_.min_; }
    double west() const { return lon_.min_; }
    double east() const { return lon_.max_; }
    bool zonal_band() const { return mir::Longitude(lon_.max_ - lon_.min_) == mir::Longitude::GLOBE; }
    operator bool() const { return true; }
};
using RectangularDomain = Domain;


struct Library {
    static Library& instance() {
        static Library me;
        return me;
    }
    void initialise(const eckit::option::CmdArgs&) {}
    void finalise() {}

private:
    Library() = default;
};


namespace util {


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


static const util::Config defaultConfig;
static const Domain globalDomain({mir::Longitude::GREENWICH.value(), mir::Longitude::GLOBE.value()},
                                 {mir::Latitude::SOUTH_POLE.value(), mir::Latitude::NORTH_POLE.value()});


namespace grid {


struct Spacing : protected std::vector<double> {
    using Spec = util::Config;
    Spec spec() const { return defaultConfig; }

    using vector = vector<value_type>;
    using vector::vector;

    using vector::back;
    using vector::front;
    using vector::size;
    using vector::operator[];

    value_type min() const { return front() < back() ? front() : back(); }
    value_type max() const { return front() > back() ? front() : back(); }
};


struct LinearSpacing : public Spacing {
    LinearSpacing(value_type a, value_type b, long n, bool endpoint = true);
    LinearSpacing() = default;
};


}  // namespace grid


namespace projection {
struct ProjectionFactory {
    static bool has(const std::string&);
};
}  // namespace projection


namespace MeshGenerator {
using Parameters = util::Config;
}


struct Projection {
    // no projection supported
    using Spec = util::Config;
    Spec spec_;
    Spec spec() const { return spec_; }
    Projection(const Spec& spec = defaultConfig);
    operator bool() const { return true; }
    mir::Point2 xy(const mir::Point2& p) const { return p; }
    mir::Point2 lonlat(const mir::Point2& p) const { return p; }
    Domain lonlatBoundingBox(const Domain& r) const { return r; }
};


using idx_t    = long;
using pl_t     = std::vector<idx_t>;
using points_t = std::vector<PointXY>;


struct Grid {
    using Spec       = util::Config;
    using Projection = atlas::Projection;
    Spec spec_;
    Spec spec() const { return spec_; }
    Grid(const Spec& spec = defaultConfig);
    Grid(const Grid&);
    operator bool() const { return false; }
    Grid& operator=(const Grid&) = default;
    Projection projection() const;
};


struct StructuredGrid : Grid {
    using XSpace = grid::Spacing;
    using YSpace = grid::Spacing;
    using Grid::Grid;
    StructuredGrid(const Grid&);
    StructuredGrid(const std::string& name, const Domain& = globalDomain);
    StructuredGrid(const XSpace& lon, const YSpace& lat, const Projection& = Projection(),
                   const Domain& = globalDomain) :
        lon_(lon), lat_(lat) {}
    idx_t nx(idx_t j) const { return pl_.at(size_t(j)); }
    idx_t nx() const;
    idx_t ny() const { return idx_t(pl_.size()); }

protected:
    StructuredGrid(const pl_t& pl, const Domain&) : pl_(pl) {}
    pl_t pl_;
    XSpace lon_;
    YSpace lat_;
};
using RegularGrid = StructuredGrid;


struct ReducedGaussianGrid : StructuredGrid {
    ReducedGaussianGrid(const std::string& name, const Domain& domain = globalDomain) : StructuredGrid(name, domain) {}
    ReducedGaussianGrid(const pl_t& pl, const Domain& domain = globalDomain) : StructuredGrid(pl, domain) {}
    ReducedGaussianGrid(const Spec& spec) : ReducedGaussianGrid(spec.getString("name")) {}
    const pl_t& nx() const { return pl_; }
};


struct RegularGaussianGrid : StructuredGrid {
    RegularGaussianGrid(const std::string& name, const Domain& domain = globalDomain) : StructuredGrid(name, domain) {}
    idx_t nx() const { return pl_.at(0); }
};


struct UnstructuredGrid : Grid {
    points_t points_;
    UnstructuredGrid(points_t&& points);
};


}  // namespace atlas


#endif
