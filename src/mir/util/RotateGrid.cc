/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/RotateGrid.h"

#include "atlas/util/Config.h"


namespace {
static double degree_to_radian = M_PI / 180.;
static double radian_to_degree = 180. * M_1_PI;
}  // anonymous namespace


namespace mir {
namespace util {


RotateGrid::RotateGrid(const eckit::geometry::LLPoint2& south_pole,
                 double  south_pole_rot_angle,
                 double lonMin)
 : south_pole_(south_pole),
   south_pole_rot_angle_(south_pole_rot_angle),
   lonmin_(lonMin),
   lonmax_(lonMin + 360.) {

    // setup atlas projection to perform rotations
    atlas::util::Config config;
    config.set("type", "rotated_lonlat");
    config.set("south_pole", std::vector<double>({ south_pole_.lon(), south_pole_.lat() }));
    config.set("rotation_angle", south_pole_rot_angle_);

    atlas_rotation_ = atlas::Projection(config);
}

// For reference, this what magics uses, it appears as if it originated from fortran code
// Unfortunately there is no reference. Tests show unrotate is broken
eckit::geometry::LLPoint2 RotateGrid::magics_rotate( const eckit::geometry::LLPoint2& point ) const
{
   double lat_y = point.lat();
   double lon_x = point.lon();

   double sin_south_pole_lat = sin(degree_to_radian*(south_pole_.lat()+90.));
   double cos_south_pole_lat = cos(degree_to_radian*(south_pole_.lat()+90.));

   double ZXMXC = degree_to_radian*(lon_x - south_pole_.lon());
   double sin_lon_decr_sp = sin(ZXMXC);
   double cos_lon_decr_sp = cos(ZXMXC);
   double sin_lat = sin(degree_to_radian*lat_y);
   double cos_lat = cos(degree_to_radian*lat_y);
   double ZSYROT = cos_south_pole_lat*sin_lat - sin_south_pole_lat*cos_lat*cos_lon_decr_sp;
   ZSYROT = std::max( std::min(ZSYROT, +1.0), -1.0 );

   double PYROT = asin(ZSYROT)*radian_to_degree;

   double ZCYROT = cos(PYROT*degree_to_radian);
   double ZCXROT = (cos_south_pole_lat*cos_lat*cos_lon_decr_sp + sin_south_pole_lat*sin_lat)/ZCYROT;
   ZCXROT = std::max( std::min(ZCXROT, +1.0), -1.0 );
   double ZSXROT = cos_lat*sin_lon_decr_sp/ZCYROT;

   double PXROT = acos(ZCXROT)*radian_to_degree;

   if( ZSXROT < 0.0)
      PXROT = -PXROT;

   return eckit::geometry::LLPoint2( PXROT, PYROT);
}

eckit::geometry::LLPoint2 RotateGrid::magics_unrotate( const eckit::geometry::LLPoint2& point ) const
{
   double lat_y = point.lat();
   double lon_x = point.lon();

   double sin_south_pole_lat = sin(degree_to_radian*(south_pole_.lat()+90.));
   double cos_south_pole_lat = cos(degree_to_radian*(south_pole_.lat()+90.));
   double cos_lon = cos(degree_to_radian*lon_x);
   double sin_lat = sin(degree_to_radian*lat_y);
   double cos_lat = cos(degree_to_radian*lat_y);
   double ZSYREG = cos_south_pole_lat*sin_lat + sin_south_pole_lat*cos_lat*cos_lon;
   ZSYREG = std::max( std::min(ZSYREG, +1.0), -1.0 );
   double PYREG = asin(ZSYREG)*radian_to_degree;
   double ZCYREG = cos(PYREG*degree_to_radian);
   double ZCXMXC = (cos_south_pole_lat*cos_lat*cos_lon - sin_south_pole_lat*sin_lat)/ZCYREG;
   ZCXMXC = std::max( std::min(ZCXMXC, +1.0), -1.0 );
   double ZSXMXC = cos_lat*sin_lat/ZCYREG;
   double ZXMXC = acos(ZCXMXC)*radian_to_degree;
   if( ZSXMXC < 0.0)
      ZXMXC = -ZXMXC;
   double PXREG = ZXMXC + south_pole_.lon();

   return eckit::geometry::LLPoint2( PXREG, PYREG);
}

eckit::geometry::LLPoint2 RotateGrid::rotate( const eckit::geometry::LLPoint2& point) const {
    return atlas_rotation_.xy(point);
}

eckit::geometry::LLPoint2 RotateGrid::unrotate( const eckit::geometry::LLPoint2& point) const {
    return atlas_rotation_.lonlat(point);
}

//-----------------------------------------------------------------------------

// *** This is only used for test comparison *****
RotgridPy::RotgridPy(double south_pole_lat, double south_pole_lon,
                 double  south_pole_rot_angle,
                 double  nPoleGridLon,
                 double lonMin)
 : south_pole_lat_(south_pole_lat),
   south_pole_lon_(south_pole_lon),
   south_pole_rot_angle_(south_pole_rot_angle),
   lonmin_(lonMin),
   lonmax_(lonMin + 360.0)
{
   double south_pole_lat_in_radians = south_pole_lat_ * degree_to_radian ;
   cossouth_pole_lat_ = cos(south_pole_lat_in_radians);
   sinsouth_pole_lat_ = sin(south_pole_lat_in_radians);

   if (nPoleGridLon != 0)
      south_pole_rot_angle_ = south_pole_lon - nPoleGridLon + 180.0;
}

std::pair<double,double> RotgridPy::transform(double lat, double lon, bool inverse) const
{
   // calculate trig terms relating to longitude
   double south_pole_lon = south_pole_lon_;

   if (inverse) {
      south_pole_lon += 180.0;
      lon += south_pole_rot_angle_;
   }

   double lon_decr_south_pole_lon = lon - south_pole_lon;
   double dlonr = lon_decr_south_pole_lon * degree_to_radian;
   double cosdlonr = cos(dlonr);
   double sindlonr = sin(dlonr);

   // likewise for latitude
   double latr = lat * degree_to_radian;
   double coslatr = cos(latr);
   double sinlatr = sin(latr);

   // now the main calculation
   std::pair<double,double> ret = rotgrid_core( cossouth_pole_lat_, sinsouth_pole_lat_,
                          cosdlonr, sindlonr,
                          coslatr, sinlatr );

   // first is long, second is lat
   double latrotr = ret.first;
   double dlonrotr = ret.second;

   double lonrot = south_pole_lon + dlonrotr * radian_to_degree;
   double latrot = latrotr * radian_to_degree;

   // Still get a very small rounding error, round to 6 decimal places
   lonrot = roundf( lonrot * 1000000.0 )/1000000.0;
   latrot = roundf( latrot * 1000000.0 )/1000000.0;

   if (!inverse)
      lonrot -= south_pole_rot_angle_;

   // put lonrot back in range
   while (lonrot < lonmin_) lonrot += 360.0;
   while (lonrot >= lonmax_) lonrot -= 360.0;

//   eckit::Log::info() << "Transform returning (" << lonrot << "," <<  latrot << ")\n";
   return std::pair<double,double>(latrot,lonrot);
}

std::pair<double,double> RotgridPy::rotgrid_core(
               double cossouth_pole_lat, double sinsouth_pole_lat,
               double cosdlon, double sindlon, double coslat, double sinlat) const
{
    double cycdx = coslat * cosdlon ;

    // Evaluate rotated longitude, use atan2 to convert back to degrees
    double dlonrot = atan2(coslat * sindlon,
                             cycdx * sinsouth_pole_lat - sinlat * cossouth_pole_lat);

    // Evaluate rotated latitude
    double sinlatrot = cycdx * cossouth_pole_lat + sinlat * sinsouth_pole_lat ;

    // Uses arc sin, to convert back to degrees
    // put in range -1 to 1 in case of slight rounding error
    //  avoid error on calculating e.g. asin(1.00000001)
    if (sinlatrot > 1.0)  sinlatrot = 1.0;
    if (sinlatrot < -1.0)  sinlatrot = -1.0;

    double latrot = asin(sinlatrot);

    return std::pair<double,double>(latrot,dlonrot);
}


} // namespace util
} // namespace mir

