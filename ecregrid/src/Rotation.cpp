/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Rotation.h"

#ifndef Point_H
#include "Point.h"
#endif

#ifndef Exception_H
#include "Exception.h"
#endif

#include "DefinitionsCache.h"

#include <fstream>
#include <sstream>

inline double deg2rad(double x) {
    return x/180.0*M_PI;
}
//inline double deg2rad(double x) { return x * 0.01745329252; }
inline double rad2deg(double x) {
    return x*180.0/M_PI;
}
//inline double rad2deg(double x) { return x * 57.295779513; }

Rotation::Rotation(double lat, double lon) :
    latPole_(lat), lonPole_(lon) {
}

Rotation::~Rotation() {
}

bool Rotation::operator==(const Rotation& r) const {
    return  same(latPole_, r.latPole_) && same(lonPole_,r.lonPole_);
}

//Normalise value such as sin or cos to range (-1,1)
inline double normalise(double x) {
    return max(min(x , 1.0), -1.0);
}

Point Rotation::rotateLatitude(const Point& p, const string& what) const {
// Y remains unchanged.
    const double FACTOR = 0.017453293;
    double angle;
    double xp = p.x(), zp = p.z();

//	double y = p.y();

    if (what == "rotate")
        angle =  90.0 + latPole_;
    else
        angle = -(90.0 + latPole_);

//	cout << "Rotation::rotateLatitude  angle " << angle << endl;

    angle = angle * FACTOR;

    double cosa = cos(angle), sina = sin(angle);

    double x =  xp * cosa + zp * sina;
    double z = -xp * sina + zp * cosa;

//	double x =  p.x() * cos(angle) + p.z() * sin(angle);
//	double z = -p.x() * sin(angle) + p.z() * cos(angle);

//	cout << "Rotation::rotateLatitude x = " << x << "  z = " << z << endl;

    return Point(x,p.y(),z,p.iIndex(),p.jIndex(),p.k1dIndex());
}

Point Rotation::rotateLongitude(const Point& p, const string& what) const {
    double angle = lonPole_;
    if (what == "rotate")
        angle = -lonPole_;

//	cout << "Rotation::rotateLongitude  angle " << angle << endl;

    double newLongitude = p.longitude() + angle;

    if(newLongitude < 0.0)
        newLongitude = newLongitude + 360.0;
    if(newLongitude >= 360.0)
        newLongitude = newLongitude - 360.0;

    return Point(p.latitude(), newLongitude,p.iIndex(),p.jIndex(),p.k1dIndex());
}

Point Rotation::unRotate(const Point& p) const {
// Rotate the rotated row points back through the original latitude rotation
    Point newLat = rotateLatitude(p, "unrotate");
//	cout << " Rotation::unRotate after rotation latitude " << newLat << endl;

// Adjust the rotated line longitudes to remove the longitude rotation

    return rotateLongitude(newLat,"unrotate");
}

Point Rotation::rotate(const Point& p) const {
// Rotate the rotated row points through the original latitude rotation
    Point aroundLat = rotateLatitude(p, "rotate");

// Adjust the rotated line longitudes to remove the longitude rotation

    return rotateLongitude(aroundLat,"rotate");
}

void Rotation::angularChange(const vector<Point>& gridPoints, vector<double>& change) const {
    /**
         Rotates a grid of points about the y-axis and calculates the
         angular change to local axes at each grid point.
    */
// epsilon 1E-5 -  tolerance used for checking proximity to +/-90.0 and 180.0

    double epsilon = 0.00001;

    double longmod = -lonPole_;
    double theta  = deg2rad(latPole_);
    double sinthe = -sin(theta);
    double costhe = -cos(theta);
//---------------------------------------
    vector<Point>::const_iterator p = gridPoints.begin(), stop = gridPoints.end();
    for( ; p != stop; ++p) {
        double zlat   = deg2rad(p->latitude());
        double sinlat = sin(zlat);
        double coslat = cos(zlat);

//		double zlon   = gridPoints[i].longitude() + longmod;
        double zlon   = p->longitude() + longmod;

        if(zlon > 180.0)
            zlon = zlon - 360.0;
        if(fabs(zlon + 180.0) < epsilon)
            zlon = 180.0;

        zlon = deg2rad(zlon);
        double sinlon = sin(zlon);
        double coslon = cos(zlon);

        /* Calculate z coordinate of point after rotation and hence
        	its latitude after rotation since: z = sin(lat)  */

        double znew  = sinthe * sinlat + costhe * coslat * coslon;
        znew  = normalise(znew);

        double latnew, ncoslat;
        if(fabs(znew - 1.0) < epsilon) {
            latnew  = 1.5707963268;
            ncoslat = 0.0;
        } else if(fabs(znew + 1.0) < epsilon) {
            latnew  = 1.5707963268;
            ncoslat = 0.0;
        } else {
            latnew  = asin(znew);
            ncoslat = cos(latnew);
        }

        /* Calculate x coordinate of point after rotation and hence
           its longitude after rotation since:
        		 x = cos(lat).cos(lon)
        		 cos(lon) = x / cos(lat)  */

        if(iszero(ncoslat))
            ncoslat = 1.0;

        double lonnew;
        double zdiv   = 1.0 / ncoslat;

        double cosnew = (sinthe*coslat*coslon  - costhe*sinlat) * zdiv;
        cosnew = normalise(cosnew);

        if(zlon >= 0)
            lonnew = fabs(acos(cosnew));
        else
            lonnew = -fabs(acos(cosnew));

        /* Use cosine rule from spherical trigonometry to find angle between
           the directions of the meridians before and after rotation. */

        double cosdel = sinthe*sinlon*sin(lonnew) + coslon*cosnew;
        cosdel = normalise(cosdel);

        double delta;
        if(-costhe*zlon >= 0)
            delta = fabs(acos(cosdel));
        else
            delta = -fabs(acos(cosdel));

        //  Change the orientation angle (keep it in the range -180 to 180)
        double pdiri = -rad2deg(delta);
        if( pdiri > 180.0 )
            pdiri = pdiri - 360.0;
        if( pdiri < -180.0 || same(-180.0,pdiri))
            pdiri = pdiri + 360.0;

        change.push_back(pdiri);
    }
}

int Rotation::match(int truncation) const {
    const string resol = getShareDir() + "/definitions/resolutions";
    ref_counted_ptr< const vector<string> > resolSpec = DefinitionsCache::get(resol);

    vector<string>::const_iterator it = resolSpec->begin();
    int truncationOut, gaussian;
    double llmin, llmax;

    while (it != resolSpec->end()) {
        istringstream in(*it);
        in >> truncationOut >> llmin >> llmax >> gaussian;

        if(truncationOut == truncation)
            return gaussian;

        it++;
    }
    throw UserError("Rotation::match " + resol);
}


void Rotation::print(ostream& out) const {
    out << "Rotation{ Latitude of South Pole =[" << latPole_ << "], Longitude of South Pole =[" << lonPole_ << "] }" ;
}
