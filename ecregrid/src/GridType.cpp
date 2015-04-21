/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "GridType.h"

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Exception_H
#include "Exception.h"
#endif

#include <iterator>
#include <algorithm>

inline bool comparer(const Point& a, const Point& b) {
    return a < b;
}

GridType::GridType(bool isGlobalwe, bool isGlobalns) :
    globalWestEast_(isGlobalwe), globalNorthSouth_(isGlobalns) {
}

GridType::~GridType() {
}

long GridType::findNorthSouthNeighbours(double wlat, int& last_j, const vector<double>& latitudes, double& north, double& south, int& n, int& s) const {
    long lastLat = latitudes.size() - 1;
    ASSERT(lastLat);

//	long lastLat = indexOfLastLat_;
    for ( int jj = 0 ; jj < lastLat ;  jj++ ) {
        int j = (jj + last_j) % lastLat;
//        if((wlat < latitudes[j] || same(wlat,latitudes[j])) && wlat > latitudes[j+1]) {
        if((wlat < latitudes[j] || same(wlat,latitudes[j])) && wlat > latitudes[j+1] ) {
            north = latitudes[j];
            south = latitudes[j+1];
            n = j;
            s = j + 1;

            last_j = j;
            return lastLat;
        }

        /*  Check poles: - Input resolution lower then output
                         - Tranformation from Gaussian to Lat-Lon
                       If any of those two cases occur value of
                       output point will be determine by two nearest points
        */
        // North pole
        if ( j == 0) {
            if(wlat > latitudes[j] || same(wlat,latitudes[j])) {
                n = -1;
                s = 0;
                north = -1;
                south = latitudes[0];
                return lastLat;
            }
        }

        // South pole
        if ( j + 1 == lastLat) {
            if(wlat < latitudes[j+1] || same(wlat,latitudes[j+1])) {
                n = lastLat;
                s = -1;
                north = latitudes[lastLat];
                south = -1;
                return lastLat;
            }
        }
    }
    if(DEBUG) {
        cout << "GridType::findNorthSouthNeighbours wlat  " << wlat << "  last latitude " << latitudes[lastLat] << endl;
        cout << "GridType::findNorthSouthNeighbours  last Lat index " << lastLat << endl;
    }

    throw WrongValue("GridType::findNorthSouthNeighbours LAT can NOT be found ", wlat);
    return -1;
}

long GridType::findWestEastNeighbours(double wlon, int& last_i, const vector<double>& longitudes, double& west, double& east, int& w, int& e) const {
    long lastLon = longitudes.size() - 1;
    ASSERT(lastLon);
    // ssp added because rotated grid can have 360.0
    if(same(wlon,360.0))
        wlon = 0.;

// ssp to secure right selection of neighbours
    wlon += ROUNDING_FACTOR;
    /*
          0       1                   0      1

    	  x               rather than        x

    	  2       3                   2      3

    */

    for ( int ii = 0 ; ii < lastLon ; ii++ ) {
        int i = (ii + last_i) % lastLon;
        double lowLon = longitudes[i];
        double upLon  = longitudes[i+1];
//		cout << "))) wlon  " << wlon << " lowLon " << lowLon << "  upLon "  <<  upLon << endl;

        if((wlon > lowLon || same(wlon,lowLon)) && wlon < upLon) {
            west = lowLon;
            east = upLon;
            w    = i;
            e    = i + 1;
            last_i = i;
            return lastLon;
        }
        if(upLon < lowLon) {
            // Wrap around cases
            // Longitude point found between longitudes[i] and 3600000
            if(wlon >= lowLon) {
                west = lowLon;
                east = upLon + 360.0;
                w    = i;
                e    = i + 1;
                last_i = i;
                return lastLon;
            }
            // Longitude point found between 0 and longitudes[i+1]
            if(wlon < upLon) {
                west = lowLon - 360.0;
                east = upLon;
                w    = i;
                e    = i + 1;
                last_i = i;
                return lastLon;
            }
        }
    }
    /*
     If grid is Global  the West-East has LAST element set EQUAL to first
     to help with recognising where an output grid "wraps around" the
     break in the input grid.
     coarser to finer grid
    */
    if(globalWestEast_) {
        if((wlon > longitudes[lastLon] || same(wlon,longitudes[lastLon])) && wlon < 360.0) {
            w    = lastLon ;
            west = longitudes[lastLon];
            e = 0;
            east = 360.0;
            return lastLon;
        }
        // Input field start shifted from 0 Nils-Gear for lsm 1km
        if(wlon < longitudes[0] || same(wlon,longitudes[0])) {
//			if(DEBUG)
//				cout << "GridType::findWestEastNeighbours SPECIAL CASE^^^^^^^^^^^^^^^^^^  " << wlon << endl;
            west = longitudes[lastLon] - 360.0;
            east = longitudes[0];
            w    = lastLon;
            e    = 0;
            return lastLon;
        }
    } else {
        // from Subarea Input case
        if(same(wlon,longitudes[lastLon])) {
            west = longitudes[lastLon-1];
            east = longitudes[lastLon];
            w    = lastLon-1;
            e    = lastLon;
            return lastLon;
        }
    }

    if(DEBUG) {
        cout << "GridType::findWestEastNeighbours wlon  " << wlon << "  last longitude " << longitudes[lastLon] << endl;
        cout << "GridType::findWestEastNeighbours  last Lon index " << lastLon << endl;
    }

    throw WrongValue("GridType::findWestEastNeighbours LON can NOT be found ", wlon);
    return -1;
}

void GridType::unRotatedArea(const vector<Point>& grid) const {
    if (getenv("UNROTATED_AREA")) {
        vector<Point>::const_iterator start = grid.begin(), end = grid.end(), ne, sw;
        ne = max_element(start,end,comparer);
        sw = min_element(start,end,comparer);
        cout << "Regular::generateGrid -- Unrotated boundaries: ne -> " << *ne << "   sw -> " << *sw << endl;
    }
}

/*
void GridType::print(ostream&) const
{
}
*/
