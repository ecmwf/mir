/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "RegularLatLonCellCentered.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Rotation_H
#include "Rotation.h"
#endif

#include <sstream>

RegularLatLonCellCentered::RegularLatLonCellCentered(double north, double west, double south, double east, double ns, double we):
    LatLon(north, west, south, east, ns, we), Regular(this), shifted_(true) {
    if(!isGlobal())
        throw UserError("RegularLatLonCellCentered::RegularLatLonCellCentered grid is NOT global! ");
}

RegularLatLonCellCentered::RegularLatLonCellCentered(const Area& other, double ns, double we):
    LatLon(other, ns, we), Regular(this), shifted_(true) {
    if(!isGlobal())
        throw UserError("RegularLatLonCellCentered::RegularLatLonCellCentered grid is NOT global! ");
}

RegularLatLonCellCentered::RegularLatLonCellCentered(const Area& other, long ns, long we):
    LatLon(other, calculateNortSouthIncrement(other.north(), other.south(), ns), calculateWestEastIncrement(we)), Regular(this), shifted_(true) {
}


RegularLatLonCellCentered::RegularLatLonCellCentered(long nptsNS, long nptsWE, bool shifted):
    LatLon(calculateNorth(nptsNS),calculateWest(nptsWE,shifted),calculateSouth(nptsNS),calculateEast(nptsWE,shifted),calculateNortSouthIncrement(nptsNS),calculateWestEastIncrement(nptsWE)), Regular(this,nptsNS,nptsWE), shifted_(shifted) {
}

RegularLatLonCellCentered::RegularLatLonCellCentered(double ns, double we, bool shifted):
    LatLon(calculateNorth(ns),calculateWest(we,shifted),calculateSouth(ns),calculateEast(we,shifted),ns,we), Regular(this), shifted_(shifted) {
}

RegularLatLonCellCentered::~RegularLatLonCellCentered() {
}

Area RegularLatLonCellCentered::setGlobalArea() const {
    if(DEBUG)
        cout << "RegularLatLonCellCentered::setGlobalArea() north: " << calculateNorth(ns_) << " south: " << calculateSouth(ns_) << " west: " << calculateWest(we_,shifted_) << " east: " << calculateEast(we_,shifted_) << endl;
    return Area(calculateNorth(ns_),calculateWest(we_,shifted_),calculateSouth(ns_),calculateEast(we_,shifted_));
}

void RegularLatLonCellCentered::generateGrid1D(vector<Point>& llgrid, const Grid& grid)  const {
    if(grid.rotated()) {
        Rotation rot(grid.latPole(),grid.lonPole());
        generateRotatedGrid(llgrid,rot);
    } else
        generateGrid(llgrid);
}

Area RegularLatLonCellCentered::setGlobalArea(double west) const {
    if(west == 0)
        return setGlobalArea();
    else if (west == -180.0)
        return Area(calculateNorth(ns_),calculateWest(west,we_,shifted_),calculateSouth(ns_),calculateEast(west,we_,shifted_));

    return setGlobalArea();
}

double RegularLatLonCellCentered::calculateNortSouthIncrement(long npts) const {
    ASSERT(npts);

    return 180.0 / npts ;
}

double RegularLatLonCellCentered::calculateWestEastIncrement(long npts) const {
    ASSERT(npts);

    return 360.0 / npts ;
}

double RegularLatLonCellCentered::calculateWestEastIncrement(double east, double west, long npts, bool globalWestEast) const {
    ASSERT(npts);

    if(globalWestEast)
        return 360.0 / npts ;

    return (east - west) / npts + 1;
}

double RegularLatLonCellCentered::calculateNortSouthIncrement(double north, double south, long npts) const {
    ASSERT(npts);

    return (north - south) / npts ;
}

double RegularLatLonCellCentered::calculateNorth(long npts) const {
    ASSERT(npts);

    return 90.0 - 180.0 / (npts * 2);
}

double RegularLatLonCellCentered::calculateSouth(long npts) const {
    ASSERT(npts);

    return -90.0 + 180.0 / (npts * 2);
}

double RegularLatLonCellCentered::calculateWest(long npts, bool shifted) const {
    ASSERT(npts);

    if(shifted)
        return 360.0 / (npts * 2);
    return 0.0;
}

double RegularLatLonCellCentered::calculateEast(long npts, bool shifted) const {
    ASSERT(npts);

    if(shifted)
        return 360 - 360.0 / (npts * 2);

    return 360 - 360.0 / npts;
}


double RegularLatLonCellCentered::calculateNorth(double ns) const {
    ASSERT(ns);

    return 90.0 - ns / 2;
}

double RegularLatLonCellCentered::calculateSouth(double ns) const {
    ASSERT(ns);

    return -90.0 + ns / 2;
}

double RegularLatLonCellCentered::calculateWest(double we, bool shifted) const {
    ASSERT(we);

    if(shifted)
        return we / 2;
    return 0.0;
}

double RegularLatLonCellCentered::calculateEast(double we, bool shifted) const {
    ASSERT(we);

    if(shifted)
        return 360 - we / 2;

    return 360 - we / 2;
}

double RegularLatLonCellCentered::calculateWest(double west, double we, bool shifted) const {
    ASSERT(we);

    if(shifted)
        return west + we / 2;
    return west;
}

double RegularLatLonCellCentered::calculateEast(double west, double we, bool shifted) const {
    ASSERT(we);

    if(shifted)
        return (west + 360.0) - we / 2;

    return (west + 360.0) - we / 2;
}

int RegularLatLonCellCentered::northSouthNumberOfPoints() const {
    return int(180.0 / ns_ + ROUNDING_FACTOR );
}

int RegularLatLonCellCentered::westEastNumberOfPoints() const {
    return int(360.0 / we_ + ROUNDING_FACTOR);
}

bool RegularLatLonCellCentered::isGlobalWestEast() const {
    return true;
}

bool RegularLatLonCellCentered::isGlobalNorthSouth() const {
    return true;
//	return ( north() + ns() / 2 + AREA_FACTOR >= 90.0);
}

void RegularLatLonCellCentered::findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex ) const {
    Regular::findWestAndEastIndex(ctx, j, west, east, westLongitudeIndex,eastLongitudeIndex);
}

string RegularLatLonCellCentered::predefinedLsmFileName() const {
    return LatLon::constructLsmFilename("RegularLatLonCellCentered_");
}

void RegularLatLonCellCentered::accumulatedDataWE(const double* data, unsigned long dataLength, vector<double>& newData) const {
    return Regular::accumulatedDataWestEast(data,dataLength,newData);
}

void RegularLatLonCellCentered::reOrderNewData(const vector<double>& data_in, vector<double>& data_out, int scMode) const {
    Regular::reOrderData(data_in, data_out, scMode);
}

Grid* RegularLatLonCellCentered::newGrid(const Grid& other) const {
    Area areaNew = areaFix(other);
//	 cout << "RegularLatLonCellCentered::newGrid areaFix " << area << endl;
    return new RegularLatLonCellCentered(areaNew, ns_,we_);
}

Grid* RegularLatLonCellCentered::newGrid(const Area& area) const {
    return new RegularLatLonCellCentered(area, ns_,we_);
}

Grid* RegularLatLonCellCentered::getGlobalGrid() const {
    return new RegularLatLonCellCentered(setGlobalArea(),ns_,we_);
}

size_t  RegularLatLonCellCentered::getGridDefinition(vector<long>& v) const {
    return Regular::getGridDefinition(v);
}

void  RegularLatLonCellCentered::getOffsets(vector<int>& offsets) const {
    return Regular::getOffsets(offsets);
}

int  RegularLatLonCellCentered::getLatitudeOffset(double lat) const {
    return Regular::getLatitudeOffset(lat);
}

long  RegularLatLonCellCentered::getLatitudeOffset(double lat, long& current) const {
    return Regular::getLatitudeOffset(lat,current);
}

long RegularLatLonCellCentered::getIndex(int i, int j) const {
    return Regular::getIndex(i,j);
}

void  RegularLatLonCellCentered::addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const {
    return Regular::addWeightsAlongLatitude(weights,weightY,i);
}

unsigned long RegularLatLonCellCentered::calculatedNumberOfPoints() const {
//	return  northSouthNumberOfPoints() * westEastNumberOfPoints() ;
    return  calculatedNumberOfPts() ;
}

void RegularLatLonCellCentered::nearestsByIndex(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const {
    nearestIndexed(where,nearest,data,scMode,missingValue,howMany);
}

double RegularLatLonCellCentered::conserving(GridContext* ctx,const Point& where, const vector<double>& data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const {
    return Regular::conserving(ctx,where,data,scMode,missingValue,outNSincrement,outWEincrement);
}

void RegularLatLonCellCentered::cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const {
    Regular::cellsAreas(areas,areasSize);
}

double RegularLatLonCellCentered::fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double ns, double we)  const {
    return Regular::fluxConserving(ctx,where,areas,inputCellSize,data,scMode,missingValue,ns,we);
}

double RegularLatLonCellCentered::averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double we) const {
    return Regular::averageWeighted(ctx,where,weights,data,scMode,missingValue,outLats,we);
}

double RegularLatLonCellCentered::averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double we) const {
    return Regular::averageWeightedLsm(ctx,where,weights,data,dataLsmIn,dataLsmOut,scMode,missingValue,outLats,we);
}

void RegularLatLonCellCentered::nearestPoints(GridContext* ctx,const Point& where,vector<FieldPoint>& result, const vector<double>& data, int scMode, int npts)  const {
    nearestPts(ctx,where,result,data,scMode,npts);
}

void RegularLatLonCellCentered::nearest4pts(GridContext* ctx, const Point& where, vector<Point>& result) const {
    nearest4(ctx,where,result);
}

void RegularLatLonCellCentered::getGridAsLatLonList(double* lats, double* lons, long* length)  const {
    generateGridAsLatLonList(lats,lons,length);
}

void RegularLatLonCellCentered::generateGrid1D(vector<Point>& llgrid)  const {
    generateGrid(llgrid);
}


void RegularLatLonCellCentered::print(ostream& out) const {
    out << "RegularLatLonCellCentered-> " ;
    LatLon::print(out);
    Regular::print(out);
}

bool RegularLatLonCellCentered::sameAs(const  RegularLatLonCellCentered& other) const {
    return ns_ == other.ns_ && we_ == other.we_;
}

