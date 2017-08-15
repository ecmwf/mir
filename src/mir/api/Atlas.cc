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

void trans_use_mpi(bool)
{
    NOTIMP;
}

void trans_init()
{
    NOTIMP;
}

void trans_finalize()
{
    NOTIMP;
}

#endif

#ifndef HAVE_ATLAS

void atlas::util::gaussian_latitudes_npole_spole(int, double*)
{
    NOTIMP;
}

double Earth::radiusInMeters()
{
    NOTIMP;
}

double Earth::radiusInKm()
{
    NOTIMP;
}

double Earth::distanceInMeters(const DummyPoint&, const DummyPoint&)
{
    NOTIMP;
}

atlas::Projection::operator bool() const
{
    NOTIMP;
}

double atlas::PointLonLat::lat() const
{
    NOTIMP;
}

double atlas::PointLonLat::lon() const
{
    NOTIMP;
}

atlas::Projection atlas::Grid::projection() const
{
    NOTIMP;
}

const std::vector<long>& atlas::Grid::nx() const
{
    NOTIMP;
}

atlas::util::Config atlas::Grid::spec() const
{
    NOTIMP;
}

atlas::Grid::operator bool() const
{
    NOTIMP;
}

#endif
