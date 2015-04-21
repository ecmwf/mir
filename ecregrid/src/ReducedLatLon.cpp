/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "ReducedLatLon.h"

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

ReducedLatLon::ReducedLatLon(double north,double west,double south,double east,double ns,double we, long* gridDef,size_t gridSize):
    LatLon(north, west, south, east, ns, we), Reduced(this,gridDef,gridSize) {
}

ReducedLatLon::ReducedLatLon(double north, double west, double south, double east, double ns, double we):
    LatLon(north, west, south, east, ns, we), Reduced(this) {
}

ReducedLatLon::ReducedLatLon(const Area& other, double ns, double we, long* gridDef,size_t gridSize):
    LatLon(other,ns, we), Reduced(this,gridDef,gridSize) {
}

ReducedLatLon::ReducedLatLon(const Area& other, double ns, double we):
    LatLon(other,ns, we), Reduced(this) {
}

ReducedLatLon::ReducedLatLon(double ns, double we, long* gridDef,size_t gridSize):
    LatLon(ns, we), Reduced(this,gridDef,gridSize) {
}

ReducedLatLon::ReducedLatLon(double ns, double we):
    LatLon(ns, we), Reduced(this) {
}

void ReducedLatLon::findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex ) const {
    Reduced::findWestAndEastIndex(ctx, j, west, east, westLongitudeIndex,eastLongitudeIndex);
}

// Without west-east increment
ReducedLatLon::ReducedLatLon(double north,double west,double south,double east,double ns, long* gridDef,size_t gridSize):
    LatLon(north, west, south, east, ns), Reduced(this,gridDef,gridSize) {
}

ReducedLatLon::ReducedLatLon(double north, double west, double south, double east, double ns):
    LatLon(north, west, south, east, ns), Reduced(this) {
}

ReducedLatLon::ReducedLatLon(const Area& other, double ns, long* gridDef,size_t gridSize):
    LatLon(other,ns), Reduced(this,gridDef,gridSize) {
}

ReducedLatLon::ReducedLatLon(const Area& other, double ns):
    LatLon(other,ns), Reduced(this) {
}

ReducedLatLon::ReducedLatLon(double ns, long* gridDef,size_t gridSize):
    LatLon(ns), Reduced(this,gridDef,gridSize) {
}

ReducedLatLon::ReducedLatLon(double ns):
    LatLon(ns), Reduced(this) {
}

ReducedLatLon::~ReducedLatLon() {
}

long ReducedLatLon::getIndex(int i, int j) const {
    return Reduced::getIndex(i,j);
}

void ReducedLatLon::accumulatedDataWE(const double* data, unsigned long dataLength, vector<double>& newData) const {
    return Reduced::accumulatedDataWestEast(data,dataLength,newData);
}

void  ReducedLatLon::addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const {
    return Reduced::addWeightsAlongLatitude(weights,weightY,i);
}

string ReducedLatLon::predefinedLsmFileName() const {
    return LatLon::constructLsmFilename("ReducedLatLon_");
}

void ReducedLatLon::reOrderNewData(const vector<double>& data_in, vector<double>& data_out, int scMode) const {
    Reduced::reOrderData(data_in, data_out, scMode);
}

bool ReducedLatLon::areValidIncrements() const {
    return ns_ > 0;
}

Area ReducedLatLon::setGlobalArea() const {
    return Area(90.0, 0, -90.0, 360.0 - ns_);
}

bool ReducedLatLon::isGlobalWestEast() const {
// ns_ beacause of reduced lat/lon
//	cout << "ReducedLatLon::isGlobalWestEast " << ns_ << "  east " <<  east() << endl;
//	return ((360.0 - east() - west() - ns_) < 0.1);
    return (east() - west() + ns_ + AREA_FACTOR >= 360.0);
}

size_t  ReducedLatLon::getGridDefinition(vector<long>& v) const {
    return Reduced::getGridDefinition(v);
}

long  ReducedLatLon::getLatitudeOffset(double lat, long& current) const {
    return Reduced::getLatitudeOffset(lat,current);
}

int  ReducedLatLon::getLatitudeOffset(double lat) const {
    return Reduced::getLatitudeOffset(lat);
}

void  ReducedLatLon::getOffsets(vector<int>& offsets) const {
    return Reduced::getOffsets(offsets);
}

void ReducedLatLon::getGridAsLatLonList(double* lats, double* lons, long* length)  const {
    generateGridAsLatLonList(lats,lons,length);
}

void ReducedLatLon::generateGrid1D(vector<Point>& llgrid)  const {
    generateGrid(llgrid);
}

void ReducedLatLon::generateGrid1D(vector<Point>& llgrid, const Grid& grid)  const {
    if(grid.rotated()) {
        Rotation rot(grid.latPole(),grid.lonPole());
        generateRotatedGrid(llgrid,rot);
    } else
        generateGrid(llgrid);
}

unsigned long ReducedLatLon::calculatedNumberOfPoints() const {
    return Reduced::calculatedNumberOfPoints();
}

int ReducedLatLon::lengthOfFirstLatitude() const {
    return Reduced::lengthOfFirstLat();
}

int ReducedLatLon::lengthOfLastLatitude() const {
    return Reduced::lengthOfLastLat();
}

int ReducedLatLon::westEastNumberOfPoints() const {
    throw NotImplementedFeature("ReducedLatLon::westEastNumberOfPoints() -> Not Defined");
}

Grid* ReducedLatLon::newGrid(const Grid& input) const {
    Area area = areaFix(input);
    return new ReducedLatLon(area,ns_,we_);
}

Grid* ReducedLatLon::newGrid(const Area& area) const {
    return new ReducedLatLon(area,ns_,we_);
}

Grid* ReducedLatLon::getGlobalGrid() const {
    return new ReducedLatLon(setGlobalArea(),ns_,we_);
}

void ReducedLatLon::nearestsByIndex(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue,int howMany) const {
    return nearestIndexed(where,nearest,data,scMode,missingValue,howMany);
}

double ReducedLatLon::conserving(GridContext* ctx,const Point& where, const vector<double>& data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const {
    return Reduced::conserving(ctx,where,data,scMode,missingValue,outNSincrement,outWEincrement);
}

void ReducedLatLon::cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const {
    Reduced::cellsAreas(areas,areasSize);
}

double ReducedLatLon::fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double ns, double we)  const {
    return Reduced::fluxConserving(ctx,where,areas,inputCellSize,data,scMode,missingValue,ns,we);
}

double ReducedLatLon::averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double we) const {
    return Reduced::averageWeighted(ctx,where,weights,data,scMode,missingValue,outLats,we);
}

double ReducedLatLon::averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double we) const {
    return Reduced::averageWeightedLsm(ctx,where,weights,data,dataLsmIn,dataLsmOut,scMode,missingValue,outLats,we);
}

void ReducedLatLon::nearestPoints(GridContext* ctx, const Point& where, vector<FieldPoint>& result,const vector<double>& data, int scMode, int npts) const {
    nearestPts(ctx,where,result,data,scMode,npts);
}

void ReducedLatLon::nearest4pts(GridContext* ctx, const Point& where, vector<Point>& result) const {
    nearest4(ctx,where,result);
}

void ReducedLatLon::print(ostream& out) const {
    out << "ReducedLatLon-> " ;
    LatLon::print(out);
    Reduced::print(out);
}

bool ReducedLatLon::sameAs(const  ReducedLatLon& other) const {
    return ns_ == other.ns_;
//	ASSERT(1==0);
}
