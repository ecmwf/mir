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


#include "mir/api/Atlas.h"
#include "eckit/exception/Exceptions.h"


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

double util::Earth::radius()
{
    NOTIMP;
}

double util::Earth::distance(const PointLonLat&, const PointLonLat&)
{
    NOTIMP;
}

double util::Earth::distance(const PointXYZ&, const PointXYZ&)
{
    NOTIMP;
}

void util::Earth::convertSphericalToCartesian(const double&, const Point2&, Point3&, double)
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
