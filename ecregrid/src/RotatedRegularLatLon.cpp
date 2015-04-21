/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "RotatedRegularLatLon.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef Interpolator_H
#include "Interpolator.h"
#endif

#ifndef Cubic12pts_H
#include "Cubic12pts.h"
#endif

#include <sstream>

RotatedRegularLatLon::RotatedRegularLatLon(double ns, double we, double latPole, double lonPole):
    LatLon(ns,we), Regular(this), rotation_(latPole,lonPole) {
}

RotatedRegularLatLon::RotatedRegularLatLon(double north, double west, double south, double east, double ns, double we, double latPole, double lonPole):
    LatLon(north, west, south, east, ns, we), Regular(this), rotation_(latPole,lonPole) {
}

RotatedRegularLatLon::RotatedRegularLatLon(const Area& other, double ns, double we, double latPole, double lonPole):
    LatLon(other, ns, we), Regular(this), rotation_(latPole,lonPole) {
}
RotatedRegularLatLon::~RotatedRegularLatLon() {
}

long  RotatedRegularLatLon::getIndex(int i, int j) const {
    return Regular::getIndex(i,j);
}

void RotatedRegularLatLon::findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex ) const {
    Regular::findWestAndEastIndex(ctx, j, west, east, westLongitudeIndex, eastLongitudeIndex);
}

void  RotatedRegularLatLon::addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const {
    return Regular::addWeightsAlongLatitude(weights,weightY,i);
}

string RotatedRegularLatLon::predefinedLsmFileName() const {
    // it is regular latlon because  interpolation takes place in unrotated area
    return LatLon::constructLsmFilename("RegularLatLon_");
}

void RotatedRegularLatLon::accumulatedDataWE(const double* data, unsigned long dataLength, vector<double>& newData) const {
    return Regular::accumulatedDataWestEast(data,dataLength,newData);
}

void RotatedRegularLatLon::reOrderNewData(const vector<double>& data_in, vector<double>& data_out, int scMode) const {
    return Regular::reOrderData(data_in, data_out, scMode);
}

double RotatedRegularLatLon::latPole() const {
    return rotation_.latPole();
}

double RotatedRegularLatLon::lonPole() const {
    return rotation_.lonPole();
}

void RotatedRegularLatLon::angularChange(vector <double>& angles) const {
    vector<Point> gridPoints;
    generateUnrotatedGrid(gridPoints,rotation_);
    rotation_.angularChange(gridPoints,angles);
}

Grid* RotatedRegularLatLon::newGrid(const Area& area) const {
    return new RotatedRegularLatLon(area, ns_,we_, latPole(), lonPole());
}

Grid* RotatedRegularLatLon::newGrid(const Grid& other) const {
    Area area = areaFix(other);
    return new RotatedRegularLatLon(area, ns_,we_, latPole(), lonPole());
}

Grid* RotatedRegularLatLon::getGlobalGrid() const {
    return new RotatedRegularLatLon(setGlobalArea(),ns_,we_, latPole(), lonPole());
}

size_t  RotatedRegularLatLon::getGridDefinition(vector<long>& v) const {
    throw NotImplementedFeature("RotatedRegularLatLon::getGridDefinition");
}

int  RotatedRegularLatLon::match(int truncation) const {
    return rotation_.match(truncation);
}

int  RotatedRegularLatLon::getLatitudeOffset(double lat) const {
    return Regular::getLatitudeOffset(lat);
}

long  RotatedRegularLatLon::getLatitudeOffset(double lat, long& current) const {
    return Regular::getLatitudeOffset(lat,current);
}

void  RotatedRegularLatLon::getOffsets(vector<int>& offsets) const {
    return Regular::getOffsets(offsets);
}

int RotatedRegularLatLon::westEastNumberOfPoints() const {
#if ECREGRID_EMOS_SIMULATION
    // according to EMOS for this grid 0 is repeated for global grids supposed to be + 1
    if(DEBUG)
        cout << "RotatedRegularLatLon::westEastNumberOfPoints " << int((east() - west() + we_) / we_ + ROUNDING_FACTOR ) << endl;

    if(isGlobal())
        return int((east() - west() + we_) / we_ + ROUNDING_FACTOR );
#endif

    return LatLon::westEastNumberOfPoints();
}

void  RotatedRegularLatLon::setGlobalWestEast( double& west, double& east, double increment) const {
#if ECREGRID_EMOS_SIMULATION
    if (west == -180.0) {
        east = 180.0;
        return;
    } else {
        west = 0;
        east =  EQUATOR;
        return;
    }
#endif

    return LatLon::setGlobalWestEast(west,east,increment);
}

Area RotatedRegularLatLon::setGlobalArea(double west) const {
#if ECREGRID_EMOS_SIMULATION
    if (same(west,-180.0))
        return Area(NORTH_POLE, -180.0, SOUTH_POLE, 180.0);
    return Area(NORTH_POLE, 0, SOUTH_POLE, EQUATOR);
#endif

    return LatLon::setGlobalArea(west);
}

Area RotatedRegularLatLon::setGlobalArea() const {
#if ECREGRID_EMOS_SIMULATION
    return Area(NORTH_POLE, 0, SOUTH_POLE, EQUATOR);
#endif
    return Area(NORTH_POLE, 0, SOUTH_POLE, EQUATOR - we_);
}

unsigned long RotatedRegularLatLon::calculatedNumberOfPoints() const {
    return  northSouthNumberOfPoints() * westEastNumberOfPoints() ;
}

void RotatedRegularLatLon::nearestsByIndex(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const {
    nearestIndexed(where,nearest,data,scMode,missingValue,howMany);
}

double RotatedRegularLatLon::conserving(GridContext* ctx,const Point& where, const vector<double>& data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const {
    return Regular::conserving(ctx,where,data,scMode,missingValue,outNSincrement,outWEincrement);
}

void RotatedRegularLatLon::cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const {
    Regular::cellsAreas(areas,areasSize);
}

double RotatedRegularLatLon::fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double ns, double we)  const {
    return Regular::fluxConserving(ctx,where,areas,inputCellSize,data,scMode,missingValue,ns,we);
}

double RotatedRegularLatLon::averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double we) const {
    return Regular::averageWeighted(ctx,where,weights,data,scMode,missingValue,outLats,we);
}

double RotatedRegularLatLon::averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double we) const {
    return Regular::averageWeightedLsm(ctx,where,weights,data,dataLsmIn,dataLsmOut,scMode,missingValue,outLats,we);
}

void RotatedRegularLatLon::nearestPoints(GridContext* ctx,const Point& where,vector<FieldPoint>& result, const vector<double>& data, int scMode, int npts)  const {
    nearestPts(ctx,where,result,data,scMode,npts);
}

void RotatedRegularLatLon::nearest4pts(GridContext* ctx, const Point& where, vector<Point>& result) const {
    nearest4(ctx,where,result);
}

void RotatedRegularLatLon::getGridAsLatLonList(double* lats, double* lons, long* length)  const {
    throw NotImplementedFeature("RotatedRegularLatLon::getGridAsLatLonList");
//    generateGridAsLatLonList(lats,lons,length);
}

void RotatedRegularLatLon::generateGrid1D(vector<Point>& llgrid)  const {
//	cout << "RotatedRegularLatLon::generateGrid1D" << endl;
    generateUnrotatedGrid(llgrid,rotation_);
}

void RotatedRegularLatLon::generateGrid1D(vector<Point>& llgrid, const Grid& grid)  const {
    generateUnrotatedGrid(llgrid,rotation_);
}

void RotatedRegularLatLon::print(ostream& out) const {
    out << "RotatedRegularLatLon {" ;
    LatLon::print(out);
    Regular::print(out);
    out << "Rotation{ Latitude of South Pole =[" << rotation_.latPole() << "], Longitude of South Pole =[" << rotation_.lonPole() << "] }" ;
}

bool RotatedRegularLatLon::sameAs(const  RotatedRegularLatLon& other) const {
    return ns_ == other.ns_ && we_ == other.we_ &&  rotation_ == other.rotation_;
//	ASSERT(1==0);
}
