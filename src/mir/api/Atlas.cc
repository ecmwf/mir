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


#include "mir/api/Atlas.h"
#include "eckit/exception/Exceptions.h"

#ifndef ATLAS_HAVE_TRANS

GathGrid_t new_gathgrid(Trans_t*)
{
    NOTIMP;
}

DistSpec_t new_distspec(Trans_t*)
{
    NOTIMP;
}

InvTrans_t new_invtrans(Trans_t*)
{
    NOTIMP;
}

VorDivToUV_t new_vordiv_to_UV()
{
    NOTIMP;
}

int trans_distspec(DistSpec_t*)
{
    NOTIMP;
}

int trans_invtrans(InvTrans_t*)
{
    NOTIMP;
}

int trans_gathgrid(GathGrid_t*)
{
    NOTIMP;
}

int trans_new(Trans_t*)
{
    NOTIMP;
}

int trans_set_trunc(Trans_t*, int)
{
    NOTIMP;
}

int trans_set_write(Trans_t*, const char*)
{
    NOTIMP;
}

int trans_setup(Trans_t*)
{
    NOTIMP;
}

int trans_delete(Trans_t*)
{
    NOTIMP;
}

int trans_set_cache(Trans_t*, const void*, size_t)
{
    NOTIMP;
}

int trans_set_resol(Trans_t*, size_t, const int*)
{
    NOTIMP;
}

int trans_set_resol_lonlat(Trans_t*, int, int)
{
    NOTIMP;
}

int trans_vordiv_to_UV(VorDivToUV_t*)
{
    NOTIMP;
}

const char* trans_error_msg(int)
{
    NOTIMP;
}

int trans_use_mpi(bool)
{
    NOTIMP;
}

int trans_init()
{
    NOTIMP;
}

int trans_finalize()
{
    NOTIMP;
}

#endif

#ifndef HAVE_ATLAS
namespace atlas {

PointLonLat::PointLonLat()
{
    NOTIMP;
}

PointLonLat::PointLonLat(double, double)
{
    NOTIMP;
}

double PointLonLat::lat() const
{
    NOTIMP;
}

double PointLonLat::lon() const
{
    NOTIMP;
}

void util::gaussian_latitudes_npole_spole(int, double*)
{
    NOTIMP;
}

double util::Earth::radiusInMeters()
{
    NOTIMP;
}

double util::Earth::radiusInKm()
{
    NOTIMP;
}

double util::Earth::distanceInMeters(const PointLonLat&, const PointLonLat&)
{
    NOTIMP;
}

double util::Earth::distanceInMeters(const PointXYZ&, const PointXYZ&)
{
    NOTIMP;
}

void util::Earth::convertGeodeticToGeocentric(const PointLonLat&, PointXYZ&, const double&, const double&)
{
    NOTIMP;
}

util::Rotation::Rotation(const PointLonLat&)
{
    NOTIMP;
}

bool util::Rotation::rotated() const
{
    NOTIMP;
}

void util::Rotation::rotate(double[]) const
{
    NOTIMP;
}

Projection::operator bool() const
{
    NOTIMP;
}

Projection Grid::projection() const
{
    NOTIMP;
}

const std::vector<long>& Grid::nx() const
{
    NOTIMP;
}

util::Config Grid::spec() const
{
    NOTIMP;
}

Grid::operator bool() const
{
    NOTIMP;
}

}  // namespace atlas
#endif
