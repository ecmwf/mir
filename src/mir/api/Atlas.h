/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef mir_api_Atlas_H
#define mir_api_Atlas_H

#include <vector>
#include <array>

#include "mir/api/mir_config.h"

#ifdef HAVE_ATLAS

#include "atlas/grid/Grid.h"
#include "atlas/projection/Projection.h"
#include "atlas/util/Earth.h"
#include "atlas/util/GaussianLatitudes.h"
#include "atlas/util/Rotation.h"

#else

#include "eckit/geometry/Point2.h"
#include "eckit/geometry/Point3.h"


namespace atlas {


typedef eckit::geometry::Point2 PointXY;
typedef eckit::geometry::Point3 PointXYZ;

struct PointLonLat : eckit::geometry::Point2 {
    PointLonLat();
    PointLonLat(double, double);
    double lat() const;
    double lon() const;
};


namespace util {
void gaussian_latitudes_npole_spole(int, double*);
struct Config {
    template<class T>
    void set(const char*, T) {}
};
struct Earth {
    static double radiusInMeters();
    static double radiusInKm();
    static double distanceInMeters(const PointLonLat&, const PointLonLat&);
    static double distanceInMeters(const PointXYZ&, const PointXYZ&);
    static void convertGeodeticToGeocentric(const PointLonLat&, PointXYZ&, const double& height = 0, const double& radius = radiusInMeters());
};
struct Rotation {
    Rotation(const PointLonLat&);
    bool rotated() const;
    void rotate(double[]) const;
    void unrotate(double[]) const;
    friend std::ostream& operator<< (std::ostream&, const Rotation&) {}
};
}



class RectangularDomain {
public:
    RectangularDomain(const std::array<double, 2>&, const std::array<double, 2>&) {}
};

struct Projection {
    Projection() {}

    template<class T>
    Projection(T) {}

    operator bool() const;

    template<class T>
    PointLonLat lonlat(T) const {
        return PointLonLat();
    }
};



class Grid {
public:

Grid(){}
  template<class T>
        Grid(T) {}

    operator bool() const;
    const std::vector<long>& nx() const;
    Projection projection() const;
    util::Config spec() const;
};


namespace grid {

struct ReducedGaussianGrid : public Grid {
    template<class T>
    ReducedGaussianGrid(T) {}

    template<class T, class U>
    ReducedGaussianGrid(T, U) {}
};


struct RegularGaussianGrid : public Grid {
    template<class T>
    RegularGaussianGrid(T) {}

    template<class T, class U>
    RegularGaussianGrid(T, U) {}
};

struct LinearSpacing {
    LinearSpacing(const std::array<double, 2>&, size_t = 0) {}
    template<class T, class U, class V, class W>
    LinearSpacing(T, U, V, W) {}

    template<class T, class U, class V>
    LinearSpacing(T, U, V) {}

};

struct StructuredGrid : public Grid {

    typedef Projection Projection;

    struct XSpace {
        template<class T>
        XSpace(T) {}

        template<class T, class U>
        XSpace(T, U) {}

        XSpace(const std::array<double, 2>&, const std::vector<long>&, bool) {}
    };
    struct YSpace {
        template<class T>
        YSpace(T) {}

        template<class T, class U>
        YSpace(T, U) {}
    };

    template<class T>
    StructuredGrid(T) {}

    template<class T, class U>
    StructuredGrid(T, U) {}
     template<class T, class U, class V, class W>
    StructuredGrid(T, U, V, W) {}
};

struct UnstructuredGrid : public Grid {
template<class T>
    UnstructuredGrid(T) {}
};

}

}


#endif


#ifdef ATLAS_HAVE_TRANS
#include "transi/trans.h"
#else

const int TRANS_SUCCESS = 0;

struct Trans_t {
    int ngptotg;
    int myproc;
    int nspec2g;
    int ngptot;
    int nspec2;
    int flt;
    int ndgl;
};

struct GathGrid_t {
    const void* rgpg;
    const void* rgp;
    int nfld;
    const void* nto;
};

struct DistSpec_t {
    void* nfrom;
    const void* rspecg;
    const void* rspec;
    int nfld;
};

struct InvTrans_t {
    int nscalar;
    const void* rspscalar;
    const void* rgp;
    int nvordiv;
    const void* rspvor;
    const void* rspdiv;
};

struct VorDivToUV_t {
    int nfld;
    int ncoeff;
    int nsmax;
    const void* rspvor;
    const void* rspdiv;
    const void* rspu;
    const void* rspv;
};

GathGrid_t new_gathgrid(Trans_t*);
DistSpec_t new_distspec(Trans_t*);
InvTrans_t new_invtrans(Trans_t*);
VorDivToUV_t new_vordiv_to_UV();

int trans_distspec(DistSpec_t*);
int trans_invtrans(InvTrans_t*);
int trans_gathgrid(GathGrid_t*);
int trans_new(Trans_t*);
int trans_set_trunc(Trans_t*, int);
int trans_set_write(Trans_t*, const char*);
int trans_setup(Trans_t*);
int trans_delete(Trans_t*);
int trans_set_cache(Trans_t*, const void*, size_t);
int trans_set_resol(Trans_t*, size_t, const int*);
int trans_set_resol_lonlat(Trans_t*, int, int);


int trans_vordiv_to_UV(VorDivToUV_t*);
const char* trans_error_msg(int);

int trans_use_mpi(bool);
int trans_init();
int trans_finalize();


#endif


#endif

