/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Regular.h"


#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef GridContext_H
#include "GridContext.h"
#endif


#ifndef Rotation_H
#include "Rotation.h"
#endif

#ifndef RegularDataHelper_H
#include "RegularDataHelper.h"
#endif

#ifndef RegularDataHelperBool_H
#include "RegularDataHelperBool.h"
#endif

#include <iterator>
#include <climits>
#include <cfloat>


static const double rad    =  4.0 * atan(1.0) / 180.0 ;
//static const double rad    =  2.0 * asin(1.0) / 180.0;
static const double radius =  6371220.0;

typedef RegularDataHelper<double> DoubleHelper;

Regular::Regular(const Grid* grid):
    GridType(grid->isGlobalWestEast(),grid->isGlobalNorthSouth()), northSouthNumberOfPoints_(grid->northSouthNumberOfPoints()), westEastNumberOfPoints_(grid->westEastNumberOfPoints()), northSouthIncrement_(grid->northSouthIncrement()), westEastIncrement_(grid->westEastIncrement()),emosRotateGlobal_(false) {
    grid->latitudes(lats_);
    generateLongitudes(westEastNumberOfPoints_, grid->westEastIncrement(), grid->west(),grid->east(), lons_);
    calculatedNumberOfPoints_ = calculatedNumberOfPts();
    lastLatitudeIndex_        = lats_.size() - 1;
    lastLongitudeIndex_       = lons_.size() - 1;
    if(grid->isGlobalWestEast() && grid->isGlobalNorthSouth())
        emosRotateGlobal_ = true;

}

Regular::Regular(const Grid* grid, long nptsNS, long nptsWE):
    GridType(grid->isGlobalWestEast(),grid->isGlobalNorthSouth()), northSouthNumberOfPoints_(nptsNS), westEastNumberOfPoints_(nptsWE), northSouthIncrement_(grid->northSouthIncrement()), westEastIncrement_(grid->westEastIncrement()),emosRotateGlobal_(false) {
    grid->latitudes(lats_);
    generateLongitudes(westEastNumberOfPoints_, grid->westEastIncrement(), grid->west(),grid->east(), lons_);
    calculatedNumberOfPoints_ = calculatedNumberOfPts();
    lastLatitudeIndex_        = lats_.size() - 1;
    lastLongitudeIndex_       = lons_.size() - 1;

    if(grid->isGlobalWestEast() && grid->isGlobalNorthSouth())
        emosRotateGlobal_ = true;
}

Regular::~Regular() {
}

long Regular::operator()(int i,int j) const {
    long p = i + j * westEastNumberOfPoints_;

    ASSERT(p >= 0);
    if(p >= (long)calculatedNumberOfPoints_) {
        throw OutOfRange(p,calculatedNumberOfPoints_);
    }

    return p;
}

void Regular::addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const {
    for (long j = 0; j < westEastNumberOfPoints_; j++) {
        weights.push_back(weightY);
    }
}

void Regular::accumulatedDataWestEast(const double* data, unsigned long dataLength, vector<double>& newData) const {
    newData.clear();
    newData.reserve(dataLength);

    long latsSize = lats_.size();
    long lonsSize = lons_.size();
    ASSERT(latsSize*lonsSize == (long)dataLength);

    long count = 0;
    for(int j = 0; j < latsSize; j++) {
        double temp = 0;
        for(int i = 0 ; i <  lonsSize; ++i) {
            temp += data[count++];
            newData.push_back(temp);
        }
    }
}

void Regular::reOrderData(const vector<double>& data_in, vector<double>& data_out, int scMode) const {
    long latsSize = lats_.size();
    long lonsSize = lons_.size();
    const size_t dataLength = data_in.size();

    ASSERT(latsSize*lonsSize == (long)dataLength);

    data_out.resize(dataLength);

    if(scMode == 2) {
        long count = 0;
        for(int j = latsSize - 1 ; j > 0; --j) {
            for(int i = 0 ; i <  lonsSize; ++i) {
                data_out[count++] = data_in[j*lonsSize + i];
            }
        }
        if(DEBUG) {
            cout << "================================================================== " << endl;
            cout << "====              Output Data Re Ordered                  ==== " << endl;
            cout << "Regular::reOrderData Output Scanning Mode is West-East North-South " << endl;
            cout << "================================================================== " << endl;
        }
        return;
    }

    if(scMode == 3) {
        long count = 0;
        for(int j = 0  ; j < latsSize; ++j) {
            for(int i = lonsSize - 1 ; i > 0; --i) {
                data_out[count++] = data_in[j*lonsSize + i];
            }
        }
        if(DEBUG) {
            cout << "================================================================== " << endl;
            cout << "====              Output Data Re Ordered                  ==== " << endl;
            cout << "Regular::reOrderData Output Scanning Mode is East-West North-South " << endl;
            cout << "================================================================== " << endl;
        }
        return;
    }

    if(scMode == 4) {
        long count = 0;
        for(int j = latsSize - 1  ; j > 0; --j) {
            for(int i = lonsSize - 1 ; i > 0; --i) {
                data_out[count++] = data_in[j*lonsSize + i];
            }
        }
        if(DEBUG) {
            cout << "================================================================== " << endl;
            cout << "====              Output Data Re Ordered                  ==== " << endl;
            cout << "Regular::reOrderData Output Scanning Mode is East-West South-North " << endl;
            cout << "================================================================== " << endl;
        }
        return;
    }

    throw WrongValue("Regular::reOrderData  Scanning Mode",scMode);
}

int Regular::getLatitudeOffset(double latitude) const {
    int size = lats_.size();
    for(int i = 0 ; i < size ; i++) {
        if(same(lats_[i],latitude))
            return i * westEastNumberOfPoints_;
    }
    throw WrongValue("Regular::getLatitudeOffset Latitude",latitude);
    return 0;
}

long Regular::getLatitudeOffset(double latitude, long& current) const {
    long i = 0;
    for(long ii = 0 ; ii < northSouthNumberOfPoints_ ; ii++) {
        i = (ii + current) % northSouthNumberOfPoints_;
        if(same(lats_[i],latitude)) {
            current = i;
            return i * westEastNumberOfPoints_;
        }
    }
    cout << "Regular::getLatitudeOffset Latitude: " << latitude  << " i " << i << " lat[i] " << lats_[i] <<  endl;
    throw WrongValue("Regular::getLatitudeOffset Latitude",latitude);
    return 0;
}

void Regular::getOffsets(vector<int>& offsets) const {
    offsets.clear();
    offsets.reserve(northSouthNumberOfPoints_);

    for(int i = 0 ; i < northSouthNumberOfPoints_ ; i++)
        offsets[i] = westEastNumberOfPoints_ * i;
}

size_t  Regular::getGridDefinition(vector<long>& gridSpec) const {
    gridSpec.clear();
    gridSpec.reserve(northSouthNumberOfPoints_);

    for (int i = 0; i < northSouthNumberOfPoints_ ; i++)
        gridSpec.push_back(westEastNumberOfPoints_);

    return northSouthNumberOfPoints_;
}

void Regular::generateLongitudes(int numberOfPoints, double increment, double west, double east, vector<double>& longitudes) const {
    longitudes.reserve(numberOfPoints);
    double longitude = west, lon = 0;


//		cout << "Regular::generateLongitudes #################" << endl;
//		cout << "Regular::generateLongitudes numberOfPoints: " << numberOfPoints << endl;
//		cout << "Regular::generateLongitudes west: " << west << "  east " << east << endl;
//		increment = floor(increment * MULT) / MULT;
    for(int i = 0 ; i < numberOfPoints ; i++) {
        if (longitude < 0 && !same(0.0, longitude))
            lon = longitude + 360.0;
        else
            lon = longitude;
        longitudes.push_back(lon);
// ssp RERR to be checked later if gives better results
        longitude += increment ;
//			longitude = west + ((i + 1) * increment) ;
    }
}

void Regular::generateGrid(vector<Point>& llgrid)  const {
    llgrid.clear();

    size_t latSize = lats_.size();
    size_t lonSize = lons_.size();
    for(size_t j = 0; j < latSize; j++) {
        for(size_t i = 0; i < lonSize; i++) {
            Point point(lats_[j],lons_[i],i,j,getIndex(i,j));
            llgrid.push_back(point);
//			cout.precision(15);
//			cout << point << endl;
        }
    }
}

void Regular::generateGridAsLatLonList(double* lats, double* lons, long* length)  const {
    ASSERT((long)calculatedNumberOfPoints_ >= *length);

    size_t latSize = lats_.size();
    size_t lonSize = lons_.size();
    long count = 0;
    for(size_t j = 0; j < latSize; j++) {
        for(size_t i = 0; i < lonSize; i++) {
            lats[count] = lats_[j];
            lons[count] = lons_[i];
            count++;
        }
    }
    *length = calculatedNumberOfPoints_;
}

void Regular::cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const {
    areas.clear();
    areasSize.clear();

    size_t latSize = lats_.size();
    size_t lonSize = lons_.size();
    double middleOfLayer = lats_[0] - lats_[1];
    for(size_t j = 0; j < latSize; j++) {
        double northOfCell = lats_[j] + middleOfLayer / 2 ;
        if(northOfCell > lats_[0])
            northOfCell = lats_[0];
        if( j+1 < latSize)
            middleOfLayer = (lats_[j] - lats_[j+1]) / 2;
        double southOfCell = lats_[j] - middleOfLayer / 2 ;
        if(southOfCell < lats_[lastLatitudeIndex_])
            southOfCell = lats_[lastLatitudeIndex_];

        for(size_t i = 0; i < lonSize; i++) {
            double westOfCell = lons_[i] - (westEastIncrement_ / 2);
            if(westOfCell < 0)
                westOfCell += 360.0;
            double eastOfCell = lons_[i] + (westEastIncrement_ / 2);
            if(eastOfCell >= 360.0)
                eastOfCell -= 360.0;
            Area area(northOfCell,westOfCell,southOfCell,eastOfCell);
//			cout << point << endl;
            areas.push_back(area);
            areasSize.push_back(area.size());
        }
    }
}

void Regular::generateUnrotatedGrid(vector<Point>& llgrid, const Rotation& rot)  const {
    llgrid.clear();

    size_t latSize = lats_.size();
    size_t lonSize = lons_.size();
#if ECREGRID_EMOS_SIMULATION
    if(emosRotateGlobal_) {
        for(int j = 0; j < latSize; j++) {
            for(int i = 0; i < lonSize+1; i++) {
                if(i == lonSize) {
                    Point point(lats_[j],360.0,0,j,getIndexRotatedGlobal(0,j));
//           		 	Point point(lats_[j],lons_[0],0,j,getIndex(0,j));
                    Point p ( rot.unRotate(point));
                    llgrid.push_back(p);
                } else {
                    Point point(lats_[j],lons_[i],i,j,getIndexRotatedGlobal(i,j));
                    Point p ( rot.unRotate(point));
                    llgrid.push_back(p);
                }
            }
        }
        return;
    }
#endif
    for(size_t j = 0; j < latSize; j++) {
        for(size_t i = 0; i < lonSize; i++) {
            Point point(lats_[j],lons_[i],i,j,getIndex(i,j));
            Point p ( rot.unRotate(point));
            llgrid.push_back(p);
        }
    }
    unRotatedArea(llgrid);
}

void Regular::generateRotatedGrid(vector<Point>& llgrid, const Rotation& rot)  const {
    llgrid.clear();

    size_t latSize = lats_.size();
    size_t lonSize = lons_.size();
    for(size_t j = 0; j < latSize; j++) {
        for(size_t i = 0; i < lonSize; i++) {
            Point point(lats_[j],lons_[i],i,j,getIndex(i,j));
            Point p ( rot.rotate(point));
            llgrid.push_back(p);
        }
    }
//	sort(llgrid.begin(), llgrid.end());
    unRotatedArea(llgrid);
}

struct RegularGridContext : public GridContext {
    int last_j_;
    int last_i_;
    double last_lat_;
    double north_;
    double south_;
    int n_;
    int s_;
    RegularGridContext() : last_j_(0), last_i_(0), last_lat_(DBL_MAX), north_(0.0), south_(0.0), n_(0), s_(0) {}
};


GridContext* Regular::getGridContext() const {
    if(DEBUG)
        cout << "Regular::getGridContext" << endl;
    return new RegularGridContext();
}

void Regular::nearestIndexed(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const {
    nearest.clear();
    DoubleHelper  values(data,scMode,missingValue,northSouthNumberOfPoints_,westEastNumberOfPoints_,calculatedNumberOfPoints_);

    int lastIIndex = westEastNumberOfPoints_ -1;
    int lastJIndex = northSouthNumberOfPoints_ -1;

    int iInd   = where.iIndex();
    int jInd   = where.jIndex();

// It assume the grid is global
    int ileft  = iInd - 1;
    if(ileft < 0)
        ileft = lastIIndex;

    int iright = iInd + 1;
    if(iright > lastIIndex)
        iright = 0;

    int jup = jInd - 1;
    if(jup < 0)
        jup = -1;

    int jdown = jInd + 1;
    if(jdown > lastJIndex)
        jdown = -1;
    /*
        Indexing of the points

                    0

                1   x   3

                    2
    */
    double value = 0;
    // 0
    if(jup != -1) {
        value = values(iInd,jup);
    } else {
        value = missingValue;
    }
    nearest.push_back( FieldPoint(lats_[jup],lons_[iInd],iInd,jup,value));
    // 1
    value = values(ileft,jInd);
    nearest.push_back( FieldPoint(lats_[jInd],lons_[ileft],ileft,jInd,value));
    // 2
    if(jdown != -1) {
        value = values(iInd,jdown);
    } else {
        value = missingValue;
    }

    nearest.push_back( FieldPoint(lats_[jdown],lons_[iInd],iInd,jdown,value));
    // 3
    value = values(iright,jInd);
    nearest.push_back( FieldPoint(lats_[jInd],lons_[iright],iright,jInd,value));
}

void Regular::nearest4(GridContext* ctx,const Point& where, vector<Point>& result)  const {
    double north, south, west, east;
    int n = -1, s = -1, w = -1, e = -1;

    RegularGridContext* c = static_cast<RegularGridContext*>(ctx);
    ASSERT(c);

    result.clear();

    double wlat = where.latitude();
    double wlon = where.longitude();

    if(!same(c->last_lat_,wlat)) {
        c->last_lat_ = wlat;
        long lastLat = findNorthSouthNeighbours(wlat,c->last_j_,lats_,north,south,n,s);
        ASSERT(lastLat == lastLatitudeIndex_);
        c->n_ = n;
        c->s_ = s;
    } else {
        n = c->n_;
        s = c->s_;
    }
    findWestEastNeighbours(wlon,c->last_i_,lons_,west,east,w,e);

    long k = -1;
// Longitude
    if(n != -1) {
        // 0
        k = getIndex(w,n);
        result.push_back( Point(lats_[n],west,w,n,k) );
        // 1
        k = getIndex(e,n);
        result.push_back( Point(lats_[n],east,e,n,k) );
    }
    if(s != -1) {
        // 2
        k = getIndex(w,s);
        result.push_back( Point(lats_[s],west,w,s,k) );
        // 3
        k = getIndex(e,s);
        result.push_back( Point(lats_[s],east,e,s,k) );
    }
}

void Regular::nearestPts(GridContext* ctx,const Point& where, vector<FieldPoint>& result, const vector<double>& data, int scMode, int howMany)  const {
    double north, south, west, east;
    int n = -1, s = -1, w = -1, e = -1;
    long nwCorner = -1, nnwCorner = -1;
    long neCorner = -1, nneCorner = -1;
    long swCorner = -1, sswCorner = -1;
    long seCorner = -1;

    RegularGridContext* c = static_cast<RegularGridContext*>(ctx);
    ASSERT(c);

    result.clear();

    double wlat = where.latitude();
    double wlon = where.longitude();

    if(!same(c->last_lat_,wlat)) {
        c->last_lat_ = wlat;
        long lastLat = findNorthSouthNeighbours(wlat,c->last_j_,lats_,north,south,n,s);
        ASSERT(lastLat == lastLatitudeIndex_);
        c->n_ = n;
        c->s_ = s;
    } else {
        n = c->n_;
        s = c->s_;
    }
    findWestEastNeighbours(wlon,c->last_i_,lons_,west,east,w,e);

    long k = -1;
// Longitude
    if(n != -1) {
        // 0
        k = get1dIndex(w,n,scMode);
        result.push_back( FieldPoint(lats_[n],west,w,n,k,data[k]) );
        // 1
        k = get1dIndex(e,n,scMode);
        result.push_back( FieldPoint(lats_[n],east,e,n,k,data[k]) );
    }
    if(s != -1) {
        // 2
        k = get1dIndex(w,s,scMode);
        result.push_back( FieldPoint(lats_[s],west,w,s,k,data[k]) );
        // 3
        k = get1dIndex(e,s,scMode);
        result.push_back( FieldPoint(lats_[s],east,e,s,k,data[k]) );
    }

    if(result.size() < 4)
        return;

    int lonIndex, latIndex;

    if (howMany > 4) {
        /*
        	Numbering of the points (I is the interpolation point):
        	   36  37   38   39      40   41   42  43

        	   63  16   17   18      19   20   21  44

               62  35   12    4       5   13   22  45

               61  34    6    0       1    7   23  46
                                 (I)
               60  33    8    2       3    9   24  47

               59  32   14   10      11   15   25  48

        	   58  31   30   29      28   27   26  49

        	   57  56   55   54      53   52   51  50

        */
        // if top northern  or southern row missing - return
        if(n - 1 < 0 || s + 1 > lastLatitudeIndex_)
            return;

        // 4
        lonIndex = w;
        latIndex = n - 1;
        k = get1dIndex(lonIndex,latIndex,scMode);
        result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );

        // 5
        lonIndex = e;
        latIndex = n - 1;
        k = get1dIndex(lonIndex,latIndex,scMode);
        result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
        // 6
        lonIndex = ifIndexLastLon(w - 1, lastLongitudeIndex_);
        latIndex = n;
        k = get1dIndex(lonIndex,latIndex,scMode);
        result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
        // 7
        lonIndex = ifIndexFirstLon(e + 1, lastLongitudeIndex_);
        latIndex = n;
        k = get1dIndex(lonIndex,latIndex,scMode);
        result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
        // 8
        lonIndex = ifIndexLastLon(w - 1, lastLongitudeIndex_);
        latIndex = s;
        k = get1dIndex(lonIndex,latIndex,scMode);
        result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
        // 9
        lonIndex = ifIndexFirstLon(e + 1, lastLongitudeIndex_);
        latIndex = s;
        k = get1dIndex(lonIndex,latIndex,scMode);
        result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
        // 10
        lonIndex = w;
        latIndex = s + 1;
        k = get1dIndex(lonIndex,latIndex,scMode);
        result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
        // 11
        lonIndex = e;
        latIndex = s + 1;
        k = get1dIndex(lonIndex,latIndex,scMode);
        result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
    }

    if (howMany > 12) {
        // 12
        lonIndex = ifIndexLastLon(w - 1, lastLongitudeIndex_);
        nwCorner = lonIndex;
        latIndex = n - 1;
        k = get1dIndex(lonIndex,latIndex,scMode);
        result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
        // 13
        lonIndex = ifIndexFirstLon(e + 1, lastLongitudeIndex_);
        neCorner = lonIndex;
        latIndex = n - 1;
        k = get1dIndex(lonIndex,latIndex,scMode);
        result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
        // 14
        lonIndex = ifIndexLastLon(w - 1, lastLongitudeIndex_);
        swCorner = lonIndex;
        latIndex = s + 1;
        k = get1dIndex(lonIndex,latIndex,scMode);
        result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
        // 15
        lonIndex = ifIndexFirstLon(e + 1, lastLongitudeIndex_);
        seCorner = lonIndex;
        latIndex = s + 1;
        k = get1dIndex(lonIndex,latIndex,scMode);
        result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
    }
    if (howMany > 16) {
        // if top northern  or southern row missing - return
        if(n - 2 < 0 || s + 2 > lastLatitudeIndex_)
            return;
        // 16 - 21
        latIndex  = n - 2;
        lonIndex  = ifIndexLastLon(nwCorner - 1, lastLongitudeIndex_);
        nnwCorner = lonIndex;
        for(int j = 0; j < 6 ; j++) {
            k = get1dIndex(lonIndex,latIndex,scMode);
            result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
            nneCorner = lonIndex;
            lonIndex  = ifIndexFirstLon(lonIndex + 1, lastLongitudeIndex_);
        }
        // 22 - 26
        lonIndex = nneCorner;
        for(int j = 0; j < 5 ; j++) {
            ++latIndex;;
            k = get1dIndex(lonIndex,latIndex,scMode);
            result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
        }
//		cout << " latIndex " << latIndex << " s + 2 " << s + 2 << endl;
        ASSERT((latIndex) == (s + 2));
        // 27 - 31
        latIndex = s + 2;
        lonIndex = nneCorner;
        for(int j = 0; j < 5 ; j++) {
            lonIndex  = ifIndexLastLon(lonIndex - 1, lastLongitudeIndex_);
            sswCorner = lonIndex;
            k = get1dIndex(lonIndex,latIndex,scMode);
            result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
        }
//		cout << " sswCorner " << sswCorner << " nnwCorner " << nnwCorner << endl;
        ASSERT(sswCorner == nnwCorner);
        // 32 - 35
        lonIndex = sswCorner;
        for(int j = 0; j < 4 ; j++) {
            --latIndex;;
            k = get1dIndex(lonIndex,latIndex,scMode);
            result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
        }
    }
    if (howMany > 36) {
        // if top northern  or southern row missing - return
        if(n - 3 < 0 || s + 3 > lastLatitudeIndex_)
            return;
        long nnneCorner = -1, ssswCorner = -1;
        long nnnwCorner = -1;
        // 36 - 43
        latIndex   = n - 3;
        lonIndex   = ifIndexLastLon(nnwCorner - 1, lastLongitudeIndex_);
        nnnwCorner = lonIndex;
        for(int j = 0; j < 8 ; j++) {
            k = get1dIndex(lonIndex,latIndex,scMode);
            result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
            nnneCorner = lonIndex;
            lonIndex   = ifIndexFirstLon(lonIndex + 1, lastLongitudeIndex_);
        }
        // 44 - 50
        lonIndex = nnneCorner;
        for(int j = 0; j < 7 ; j++) {
            ++latIndex;;
            k = get1dIndex(lonIndex,latIndex,scMode);
            result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
        }
        ASSERT((latIndex) == (s + 3));
        // 51 - 57
        latIndex = s + 3;
        lonIndex = nnneCorner;
        for(int j = 0; j < 7 ; j++) {
            lonIndex   = ifIndexLastLon(lonIndex - 1, lastLongitudeIndex_);
            ssswCorner = lonIndex;
            k = get1dIndex(lonIndex,latIndex,scMode);
            result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
        }
        ASSERT(ssswCorner == nnnwCorner);
        // 58 - 63
        lonIndex = ssswCorner;
        for(int j = 0; j < 6 ; j++) {
            --latIndex;;
            k = get1dIndex(lonIndex,latIndex,scMode);
            result.push_back( FieldPoint(lats_[latIndex],lons_[lonIndex],lonIndex,latIndex,k,data[k]) );
        }
    }

}

double Regular::conserving(GridContext* ctx,const Point& where, const vector<double>& data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const {
    RegularGridContext* c = static_cast<RegularGridContext*>(ctx);
    ASSERT(c);

    int last_j = c->last_j_;

    double wlat = where.latitude();
    double wlon = where.longitude();

    double northOfCell = wlat + outNSincrement / 2 ;
    if(northOfCell > 90.0)
        northOfCell = 90.0;

    double southOfCell = wlat - outNSincrement / 2 ;
    if(southOfCell < -90.0)
        southOfCell = -90.0;

    double northNorth = 0, southNorth = 0;
    double northSouth = 0, southSouth = 0;
    int nn = -1, sn = -1;
    int ns = -1, ss = -1;

    // up boundary of cell
    long lastLat = findNorthSouthNeighbours(northOfCell,last_j,lats_,northNorth,southNorth,nn,sn);
    c->last_j_ = last_j;
    ASSERT(lastLat == lastLatitudeIndex_);

    // down boundary of cell
    lastLat = findNorthSouthNeighbours(southOfCell,last_j,lats_,northSouth,southSouth,ns,ss);

    long startJ = nn;
    if (startJ < 0)
        startJ = 0;

    long latSize = lastLatitudeIndex_ + 1;
    long endJ = ss + 1;
    if ( endJ > latSize )
        endJ = latSize;

    double westOfCell = wlon - outWEincrement / 2;
    double eastOfCell = wlon + outWEincrement / 2;

    double westWest = 0, eastWest = 0;
    double westEast = 0, eastEast = 0;

    vector<double> valuesAlongMeridian;
    valuesAlongMeridian.reserve(endJ - startJ);

    double accumulate = 0;
    /* ssp to solve the problem  when west boundary less than 360 and east boundary > 0
       in that case left value is much higher than right
    */
    int ww = -1, ew = -1;
    int we = -1, ee = -1;

    int last_i =  c->last_i_;
    // left boundary of cell
    long indexOfLastLon = findWestEastNeighbours(westOfCell,last_i,lons_,westWest,eastWest,ww,ew);
    c->last_i_ = last_i;
    // right boundary of cell
    indexOfLastLon = findWestEastNeighbours(eastOfCell,last_i,lons_,westEast,eastEast,we,ee);
    /*
    if(bicubic)
    	int eew  = ifIndexFirstLon(ew + 1, indexOfLastLon);
    	int www  = ifIndexLastLon(ww - 1, indexOfLastLon);
    */

    for( long j = startJ ; j < endJ ; j++) {

        // left boundary of cell
        long kww = get1dIndex(ww,j,scMode);
        long kew = get1dIndex(ew,j,scMode);
//		cout << "west: " << westOfCell << " j " << j << " westwest: " << westWest  << " eastwest " << eastWest << endl;
        double leftValue = linear(westOfCell,westWest,data[kww],eastWest,data[kew],missingValue);
//		cout << "west: " << westOfCell << " j " << j << " value: " << leftValue  << " data w: " << data[kww] << " data e: " << data[kew] << endl;

        // right boundary of cell
        long kwe = get1dIndex(we,j,scMode);
        long kee = get1dIndex(ee,j,scMode);
        double rightValue = linear(eastOfCell,westEast,data[kwe],eastEast,data[kee],missingValue);

        accumulate += rightValue - leftValue;
        valuesAlongMeridian.push_back(accumulate);
    }
    int lastIndex = valuesAlongMeridian.size() - 1;

    double upValue = linear(northOfCell,northNorth,valuesAlongMeridian[0],southNorth,valuesAlongMeridian[1],missingValue);
//		cout << "northOfCell: " << northOfCell << " northNorth: " << northNorth << " southNorth: " << southNorth << " upValue " << upValue << endl;

    double downValue = linear(southOfCell,northSouth,valuesAlongMeridian[lastIndex-1],southSouth,valuesAlongMeridian[lastIndex],missingValue);

//		cout << "southOfCell: " << southOfCell << " northSouth: " << northSouth << " southSouth: " << southSouth << " downValue " << downValue << endl;

//		cout << "VALUE " << downValue - upValue << endl;

    return downValue - upValue;
}

double Regular::averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double outWEincrement)  const {
    long startJ  = 0, endJ  = 0;
    long startI  = 0, endI  = 0;
    long startII = 0, endII = 0;

    bool flux = false;
    Area a = cell(ctx,flux,where,outLats,outWEincrement,startJ,endJ,startI,endI,startII,endII);

    ASSERT(startI >= 0 && endI <= lastLongitudeIndex_ + 1);
    long count = 0;

    double sum = 0, summ = 0;
    for( long j = startJ ; j < endJ ; j++) {
        for( int i = startI ; i < endI ; i++) {
            long k = get1dIndex(i,j,scMode);
            double value = data[k];
            if (!same(value,missingValue)) {
                double weight = weights[k];
                sum += value * weight;
                summ += weight;
                count++;
            }
        }
        if(endII) {
            for( long i = startII ; i < endII; i++) {
                long k = get1dIndex(i,j,scMode);
                double value = data[k];
                if (!same(value,missingValue)) {
                    double weight = weights[k];
                    sum += value * weight;
                    summ += weight;
                    count++;
                }
            }
        }
    }

    if(!count) {
        cout << "Reduced::averageWeighted startJ: " << startJ << " endJ: " << endJ << endl;
        cout << "----------------------------------------------------" << endl;
        return missingValue;
    }

    if(iszero(sum))
        return 0;

    return sum / summ;
}

double Regular::fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double outNS, double outWEincrement)  const {
    long startJ  = 0, endJ  = 0;
    long startI  = 0, endI  = 0;
    long startII = 0, endII = 0;

    bool flux = true;
    vector<double> outLats;
    Area outCellArea = cell(ctx,flux,where,outLats,outWEincrement,startJ,endJ,startI,endI,startII,endII);

    ASSERT(startI >= 0 && endI <= lastLongitudeIndex_ + 1);

    double outValue = 0;
    double areaCheck = 0;
    for( long j = startJ ; j < endJ ; j++) {
        for( int i = startI ; i < endI ; i++) {
            long k = get1dIndex(i,j,scMode);
            double value = data[k];
            if (!same(value,missingValue)) {
                if(outCellArea.contain(areas[k])) {
                    outValue += inputCellSize[k] * value / outCellArea.size();
                    areaCheck += inputCellSize[k] / outCellArea.size();
                } else if(outCellArea.intersect(areas[k])) {
                    Area common = outCellArea.intersection(areas[k]);
                    outValue += common.size() * value / outCellArea.size();
                }
            }
        }
        if(endII) {
            for( long i = startII ; i < endII; i++) {
                long k = get1dIndex(i,j,scMode);
                double value = data[k];
                if (!same(value,missingValue)) {
                    if(outCellArea.contain(areas[k])) {
                        outValue += inputCellSize[k] * value / outCellArea.size();
                        areaCheck += inputCellSize[k] / outCellArea.size();
                    } else if(outCellArea.intersect(areas[k])) {
                        Area common = outCellArea.intersection(areas[k]);
                        outValue += common.size() * value / outCellArea.size();
                    }
                }
            }
        }
    }

    return outValue;
}

double Regular::averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double outWEincrement)  const {
    long startJ  = 0, endJ  = 0;
    long startI  = 0, endI  = 0;
    long startII = 0, endII = 0;

    Area a = cell(ctx,false,where,outLats,outWEincrement,startJ,endJ,startI,endI,startII,endII);

    ASSERT(startI >= 0 && endI <= lastLongitudeIndex_ + 1);

    double sum = 0, summ = 0;
    for( long j = startJ ; j < endJ ; j++) {
        for( int i = startI ; i < endI ; i++) {
            long k = get1dIndex(i,j,scMode);
            double value = data[k];
            if (!same(value,missingValue)) {
                double weight = weights[k];
                if(dataLsmOut[where.k1dIndex()] != dataLsmIn[k])
                    weight *= LSM_FACTOR;
                sum += value * weight;
                summ += weight;
            }
        }
        if(endII) {
            for( long i = startII ; i < endII; i++) {
                long k = get1dIndex(i,j,scMode);
                double value = data[k];
                if (!same(value,missingValue)) {
                    double weight = weights[k];
                    if(dataLsmOut[where.k1dIndex()] != dataLsmIn[k])
                        weight *= LSM_FACTOR;
                    sum += value * weight;
                    summ += weight;
                }
            }
        }
    }

    if(iszero(sum))
        return 0;

    return sum / summ;
}

void Regular::findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex )  const {
    RegularGridContext* c = static_cast<RegularGridContext*>(ctx);
    ASSERT(c);
    int ww = 0, ee = 0;
    double westWest = 0, eastEast = 0, westEast = 0, eastWest = 0;
    // left boundary
    long indexOfLastLon = findWestEastNeighbours(west,c->last_i_,lons_,westWest,eastWest,westLongitudeIndex,ww);
    // right boundary
    indexOfLastLon = findWestEastNeighbours(east,c->last_i_,lons_,westEast,eastEast,ee,eastLongitudeIndex);
//	long indexOfLastLon = findWestEastNeighbours(west,c->last_i_,lons_,westWest,eastWest,ww,westLongitudeIndex);
//	indexOfLastLon = findWestEastNeighbours(east,c->last_i_,lons_,westEast,eastEast,eastLongitudeIndex,ee);
}
Area Regular::cell(GridContext* ctx, bool flux, const Point& where, const vector<double>& outLats, double outWEincrement, long& startJ, long& endJ, long& startI, long& endI, long& startII, long& endII) const {
    RegularGridContext* c = static_cast<RegularGridContext*>(ctx);
    ASSERT(c);

    double wlat = where.latitude();
    double wlon = where.longitude();
    int outLastIndex = outLats.size() - 1;

    double northOfCell = 90.0 ;
    double southOfCell = -90.0 ;

    int jOutIndex = where.jIndex();

    long lonSize = lastLongitudeIndex_ + 1;

    if(!same(c->last_lat_,wlat)) {
        c->last_lat_ = wlat;
        int last_j = c->last_j_;

        bool northPole = false;
        bool southPole = false;

//	cout << "-------------- "  << endl;
//		cout << "out index " << jOutIndex << " last index " << outLastIndex << endl;

        if(jOutIndex) {
            northOfCell = wlat + ((outLats[jOutIndex - 1] - outLats[jOutIndex]) * 0.5);
            //	cout << "northOfCell " << northOfCell << " gore " << outLats[jOutIndex -1] << " dole " << outLats[jOutIndex] << endl;

        }

        if(northOfCell > lats_[0] || jOutIndex == 0) {
            northOfCell = lats_[0];
            northPole   = true;
        }

        if(jOutIndex < outLastIndex) {
            southOfCell = wlat - ((outLats[jOutIndex] - outLats[jOutIndex + 1] ) * 0.5);
        }

        if(southOfCell < lats_[lastLatitudeIndex_] || jOutIndex == outLastIndex) {
            southOfCell = lats_[lastLatitudeIndex_];
            southPole   =  true;
            // cout << southOfCell << " last " << lats_[lastLatitudeIndex_] << endl;
        }

        if(northPole) {
            if(southOfCell > northOfCell)
                southOfCell = northOfCell;
        }
        if(southPole) {
            if(southOfCell > northOfCell)
                northOfCell = southOfCell;
        }

        c->north_ = northOfCell;
        c->south_ = southOfCell;

        double northNorth = 0, southNorth = 0;
        double northSouth = 0, southSouth = 0;
        int nn = -1, sn = -1;
        int ns = -1, ss = -1;
        // up boundary of cell
        long lastLat = findNorthSouthNeighbours(northOfCell,last_j,lats_,northNorth,southNorth,nn,sn);
        ASSERT(lastLat == lastLatitudeIndex_);

        // down boundary of cell
        lastLat = findNorthSouthNeighbours(southOfCell,last_j,lats_,northSouth,southSouth,ns,ss);
        c->last_j_ = last_j;

// for Flux Conserving north and south neighbours are out of the cell rather then inside
        if(flux) {
            startJ = nn;
            endJ = ss + 1;
        } else {
            startJ = sn;
            endJ = ns + 1;
        }

        if (startJ < 0)
            startJ = 0;

        long latSize = lastLatitudeIndex_ + 1;
//    	if ( endJ > latSize )
        if ( endJ > latSize || endJ <= 0)
            endJ = latSize;
// ----------------------------------------
        if(startJ == 0) {
            if((endJ <= startJ))
                endJ = 1;
        } else if(endJ == latSize) {
            if((endJ <= startJ))
                startJ = endJ - 1;
        }
// ----------------------------------------

        c->n_ = startJ;
        c->s_ = endJ;
//	cout << " out Lat: " << wlat << endl;
//	cout << "north " << c->north_ <<  " south   " << c->south_ << endl;
//	cout << "Number of lats: " << endJ - startJ << endl;
    } else {
        startJ = c->n_;
        endJ   = c->s_;
    }

    double westOfCell = wlon - (outWEincrement * 0.5);
    if(westOfCell < 0)
        westOfCell += 360.0;
    double eastOfCell = wlon + (outWEincrement * 0.5);
    if(eastOfCell >= 360.0)
        eastOfCell -= 360.0;
    double westEast = 0, eastEast = 0;
    double westWest = 0, eastWest = 0;
    int ww = -1, ew = -1;
    int we = -1, ee = -1;

    // left boundary of cell
    long indexOfLastLon = findWestEastNeighbours(westOfCell,c->last_i_,lons_,westWest,eastWest,ww,ew);
    // right boundary of cell
    indexOfLastLon = findWestEastNeighbours(eastOfCell,c->last_i_,lons_,westEast,eastEast,we,ee);

// out of the cell for Flux Conserving
    if(flux) {
        startI = ww;
        endI   = ee + 1;
    } else {
// those two used just to simulate Geir's selection of points
//	   	startI = ww;
//		endI   = we;
        startI = ew;
        endI   = we + 1;
    }

    bool wrap = false;

    if ( endI > lonSize) {
        wrap    = true;
        startII = 0;
        endII   = endI - lonSize;
        endI    = lonSize;
    }

    if (startI >  endI) {
        wrap   = true;
        endII   = endI;
        startII = 0;
        endI   = lonSize;
    }
//	cout << " west " << westOfCell <<  " east  " << eastOfCell << " start " << startI << " end " << endI << endl;

    return Area(c->north_,westOfCell,c->south_,eastOfCell);
}

void Regular::print(ostream& out) const {
    if(getenv("ECREGRID_PRINT_LATLON_REGULAR")) {
        out << " Regular: " << endl;
        copy(lats_.begin(), lats_.end(), ostream_iterator<double>(out, "\n"));
        copy(lons_.begin(), lons_.end(), ostream_iterator<double>(out, "\n"));
    }
}
