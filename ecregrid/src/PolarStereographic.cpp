/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "PolarStereographic.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Area_H
#include "Area.h"
#endif

#ifndef Tokenizer_H
#include <eckit/parser/Tokenizer.h>
#endif

#include <sstream>
#include <iterator>
#include <algorithm>

#include "DefinitionsCache.h"

static double rad2deg = 180.0 / M_PI;
static double deg2rad = 1 / rad2deg;

static const double rad    =  4.0 * atan(1.0) / 180.0 ;
static const double radius =  6371220.0;

inline bool comparer(const Point& a, const Point& b) {
    return a < b;
}

PolarStereographic::PolarStereographic(int areaNumber) :
    Projection() {
    standardAreaDefinitions(areaNumber);
}

PolarStereographic::PolarStereographic(const vector<Point>& llgrid, double north, double west, double south, double east, double standardLatitude, double centralLongitude, double dxInMetres, double dyInMetres, int xCoordinateOfPole, int yCoordinateOfPole, int xDirectionNumberOfPoints, int yDirectionNumberOfPoints, bool northPole) :
    Projection(llgrid,north, west, south, east),
    standardLatitude_(standardLatitude),
    dxInMetres_(dxInMetres),
    dyInMetres_(dyInMetres),
    centralLongitude_(centralLongitude),
    xCoordinateOfPole_(xCoordinateOfPole),
    yCoordinateOfPole_(yCoordinateOfPole),
    xDirectionNumberOfPoints_(xDirectionNumberOfPoints),
    yDirectionNumberOfPoints_(yDirectionNumberOfPoints),
    northPole_(northPole) {
}

PolarStereographic::~PolarStereographic() {
}

bool PolarStereographic::sameAs(const  PolarStereographic& other) const {
    return standardLatitude_ == other.standardLatitude_ &&
           centralLongitude_ == other.centralLongitude_ &&
           dxInMetres_ == other.dxInMetres_ &&
           dyInMetres_ == other.dyInMetres_ &&
           xCoordinateOfPole_ == other.xCoordinateOfPole_ &&
           yCoordinateOfPole_ == other.yCoordinateOfPole_ &&
           xDirectionNumberOfPoints_ == other.xDirectionNumberOfPoints_ &&
           yDirectionNumberOfPoints_ == other.yDirectionNumberOfPoints_ &&
           northPole_ == other.northPole_;
}

string PolarStereographic::predefinedLsmFileName() const {
    stringstream lsmFile;
    lsmFile << "PolarStereographic_";

    return lsmFile.str();
}

Grid* PolarStereographic::newGrid(const Area& other) const {
    throw NotImplementedFeature("PolarStereographic::newGrid");
}

Grid* PolarStereographic::newGrid(const Grid& other) const {
    vector<Point> llgrid;
    if(area().empty()) {
        generateGrid(llgrid);
        vector<Point>::const_iterator start = llgrid.begin(), end = llgrid.end(), ne, sw;
        ne = max_element(start,end,comparer);
        sw = min_element(start,end,comparer);
        if(DEBUG)
            cout << "PolarStereographic::newGrid boundaries: ne -> " << *ne << "   sw -> " << *sw << endl;
        /*
        */
//		Area area = generateGrid(llgrid);
//	 	return new PolarStereographic(llgrid, area.north(), area.west(), area.south(), area.east(), standardLatitude_, centralLongitude_, incrementInMetresStandardLatitude_, xCoordinateOfPole_, yCoordinateOfPole_, xDirectionNumberOfPoints_, yDirectionNumberOfPoints_, northPole_ );
        return new PolarStereographic(llgrid, ne->latitude(),sw->longitude(),sw->latitude(),ne->longitude(), standardLatitude_, centralLongitude_, dxInMetres_, dyInMetres_, xCoordinateOfPole_, yCoordinateOfPole_, xDirectionNumberOfPoints_, yDirectionNumberOfPoints_, northPole_ );
    }
    throw WrongValue("PolarStereographic::newGrid Area is not empty", 0.0);
    return 0;
}
/*
	}
	else{
	}

*/
void PolarStereographic::generateGridFirstLatLon(vector<Point>& llgrid) const {
    double d2r = acos(0.0) / 90.0;
    double pi4 = acos(0.0) / 2.0;

    double lambda0  = d2r * standardLatitude_;
    double latFirst = area_.north() * d2r;
    double lonFirst = area_.west()  * d2r;


    /* compute xFirst,yFirst in metres */
    double sinphi     = sin(latFirst);
    double cosphi     = cos(latFirst);
    double cosdlambda = cos(lonFirst - lambda0);
    double sindlambda = sin(lonFirst - lambda0);
    double kp         = radius * 2.0 * tan(pi4 - M_PI / 2);

    double xFirst =  kp * cosphi * sindlambda;
    double yFirst = -kp * cosphi * cosdlambda;

    double y = yFirst;
    for (int j = 0; j < yDirectionNumberOfPoints_; j++) {
        double x = xFirst;
        for (int i = 0; i < xDirectionNumberOfPoints_; i++) {
            double rho  = sqrt(x*x + y*y);
            double c    = 2*atan2(rho,(2.0*radius));
            double cosc = cos(c);
            double sinc = sin(c);
            //double lat  = asin(cosc * sinphi + y * sinc * cosphi / rho) / d2r;
            double lon  = (lambda0 + atan2(x * sinc, rho * cosphi * cosc - y * sinphi * sinc)) / d2r;
            if (lon < 0)
                lon += 360.0;

            x += dxInMetres_;
        }
        y += dyInMetres_;
    }

}

void PolarStereographic::generateGrid(vector<Point>& llgrid) const {
    llgrid.clear();

    int isign = 1;
    if(!northPole_)
        isign = -1;

    double zre = (radius / dxInMetres_) * (1.0 + sin(isign * standardLatitude_ * deg2rad));
    double zta = zre * zre;
    double zfact = isign * rad2deg;
    double latitude  = -90.0, latmax = -90.0;
    double longitude = 360.0, lonmin = 360.0;

    for(int j = 0; j < yDirectionNumberOfPoints_; j++) {
        int jj = j + 1;
        for(int i = 0; i < xDirectionNumberOfPoints_; i++) {
//			long ij    = j * xDirectionNumberOfPoints_ + (i + 1);

            int ii = i + 1;
            double zt2 = (ii - xCoordinateOfPole_) * (ii - xCoordinateOfPole_) + (jj - yCoordinateOfPole_) * (jj - yCoordinateOfPole_);

            double zra = sqrt(zt2);
            double zy1 = (zta - zt2) / (zta + zt2);

            if(zy1 >= 1.0)
                latitude  = 90.0;
            else if( zy1 <= -1.0)
                latitude  = -90.0;
            else
                latitude = zfact * asin(zy1);

            double zx1 = 0;
            if(zra < AREA_FACTOR)
                longitude = centralLongitude_;
            else {
                zx1  = (jj - yCoordinateOfPole_) / zra;
                if(zx1 >= 1.0)
                    longitude = 0.0;
                else if( zx1 <= -1.0)
                    longitude = 180.0;
                else {
                    longitude = acos(zx1) * rad2deg;
                }
                if(ii < xCoordinateOfPole_ && northPole_)
                    longitude = 360.0 - longitude;
                if(ii > xCoordinateOfPole_ && !northPole_)
                    longitude = 360.0 - longitude;
            }
            longitude += centralLongitude_;
            if(longitude > 180.0)
                longitude = longitude - 360.0;

            if(latmax < latitude)
                latmax = latitude;
            if(lonmin > longitude)
                lonmin = longitude;
            Point point(latitude,longitude,i,j);
//			if(latitude == 90.0 || longitude == 0)
//				cout << "j: " << j << "  i: " << i << "  p: " << point << endl;
            llgrid.push_back(point);
        }
    }
    if(DEBUG)
        cout << "PolarStereographic::generateGrid latmax : " << latmax << "  lonmin: " << lonmin << endl;

}

void PolarStereographic::standardAreaDefinitions(int areaNumber) {
    eckit::Tokenizer tokens(" ");
    string path = getShareDir() + "/definitions";

    stringstream s;
    s << path << "/standard_stereographic" ;

    ref_counted_ptr< const vector<string> > defs = DefinitionsCache::get(s.str());

    vector<string>::const_iterator it = defs->begin();

    while (it != defs->end()) {

        vector<string> v;
        tokens(*it, v);

        if (v.size() > 1) {
            if ( atoi(v[0].c_str()) == areaNumber) {
                xDirectionNumberOfPoints_ = atoi(v[1].c_str());
                yDirectionNumberOfPoints_ = atoi(v[2].c_str());
                dxInMetres_               = atof(v[3].c_str()) * 1000.0;
                dyInMetres_               = atof(v[4].c_str()) * 1000.0;
                xTopLeft_                 = atoi(v[5].c_str());
                yTopLeft_                 = atoi(v[6].c_str());
                xCoordinateOfPole_        = atoi(v[7].c_str());
                yCoordinateOfPole_        = atoi(v[8].c_str());
                centralLongitude_         = 0.0;
            }
        }

        it++;
    }

    if(DEBUG) {
        cout << "PolarStereographic::standardAreaDefinitions Area Number"  << areaNumber << endl;
        cout << "x Num Pts: "  << xDirectionNumberOfPoints_ << " y Num Pts: " << yDirectionNumberOfPoints_ << endl;
    }
}

void PolarStereographic::print(ostream& out) const {
    out << "PolarStereographic{ " ;
    Grid::print(out);
    out << endl << " Standard Latitude =[" << standardLatitude_ << "], Central Longitude =[" << centralLongitude_ << "]," << endl << " Dx =[" << dxInMetres_ << "], Dy =[" << dxInMetres_ << "]," << endl << " X Coordinate Of Pole =[" << xCoordinateOfPole_ << "], Y Coordinate Of Pole =[" << yCoordinateOfPole_ << "]," << endl << " X Direction Number Of Points =[" << xDirectionNumberOfPoints_ << "], Y Direction Number Of Points =[" << yDirectionNumberOfPoints_ << "]," << endl << " is North Pole =[" << northPole_ << "]";
}

