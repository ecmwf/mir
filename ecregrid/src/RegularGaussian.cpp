/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "RegularGaussian.h"

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

RegularGaussian::RegularGaussian(double north, double west, double south, double east, int n):
    Gaussian(north, west, south, east, n), Regular(this) {
}

RegularGaussian::RegularGaussian(int n):
    Gaussian(n), Regular(this) {
}

RegularGaussian::RegularGaussian(const Area& other, int n):
    Gaussian(other, n), Regular(this) {
}

RegularGaussian::~RegularGaussian() {
}

void  RegularGaussian::addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const {
    return Regular::addWeightsAlongLatitude(weights,weightY,i);
}

long RegularGaussian::getIndex(int i, int j) const {
    return Regular::getIndex(i,j);
}

string RegularGaussian::predefinedLsmFileName() const {
    stringstream lsmFile;
    lsmFile << "RegularGaussian_" << gaussianNumber_;

    return lsmFile.str();
}

void RegularGaussian::accumulatedDataWE(const double* data, unsigned long dataLength, vector<double>& newData) const {
    return Regular::accumulatedDataWestEast(data,dataLength,newData);
}

void RegularGaussian::reOrderNewData(const vector<double>& data_in, vector<double>& data_out, int scMode) const {
    Regular::reOrderData(data_in, data_out, scMode);
}

Grid* RegularGaussian::newGrid(const Grid& input) const {
    Area area = areaFix(input);
    return new RegularGaussian(area,gaussianNumber_);
}

Grid* RegularGaussian::newGrid(const Area& area) const {
    return new RegularGaussian(area,gaussianNumber_);
}

Grid* RegularGaussian::getGlobalGrid() const {
    return new RegularGaussian(setGlobalArea(),gaussianNumber_);
}

size_t  RegularGaussian::getGridDefinition(vector<long>& v) const {
    return Regular::getGridDefinition(v);
}

int  RegularGaussian::getLatitudeOffset(double lat) const {
    return Regular::getLatitudeOffset(lat);
}

long  RegularGaussian::getLatitudeOffset(double lat, long& current) const {
    return Regular::getLatitudeOffset(lat,current);
}

void  RegularGaussian::getOffsets(vector<int>& offsets) const {
    return Regular::getOffsets(offsets);
}

unsigned long RegularGaussian::calculatedNumberOfPoints() const {
    return westEastNumberOfPoints() * northSouthNumberOfPoints();
}

void RegularGaussian::nearestsByIndex(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const {
    nearestIndexed(where,nearest,data,scMode,missingValue,howMany);
}

double RegularGaussian::conserving(GridContext* ctx,const Point& where, const vector<double>& data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const {
    return Regular::conserving(ctx,where,data,scMode,missingValue,outNSincrement,outWEincrement);
}

void RegularGaussian::cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const {
    Regular::cellsAreas(areas,areasSize);
}

double RegularGaussian::fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double ns, double we)  const {
    return Regular::fluxConserving(ctx,where,areas,inputCellSize,data,scMode,missingValue,ns,we);
}

double RegularGaussian::averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double we) const {
    return Regular::averageWeighted(ctx,where,weights,data,scMode,missingValue,outLats,we);
}

double RegularGaussian::averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double we) const {
    return Regular::averageWeightedLsm(ctx,where,weights,data,dataLsmIn,dataLsmOut,scMode,missingValue,outLats,we);
}

void RegularGaussian::nearestPoints(GridContext* ctx,const Point& where,vector<FieldPoint>& result, const vector<double>& data, int scMode, int npts)  const {
    nearestPts(ctx,where,result,data,scMode,npts);
}

void RegularGaussian::nearest4pts(GridContext* ctx, const Point& where, vector<Point>& result) const {
    nearest4(ctx,where,result);
}

void RegularGaussian::getGridAsLatLonList(double* lats, double* lons, long* length)  const {
    generateGridAsLatLonList(lats,lons,length);
}

void RegularGaussian::generateGrid1D(vector<Point>& llgrid)  const {
    generateGrid(llgrid);
}

void RegularGaussian::findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex ) const {
    Regular::findWestAndEastIndex(ctx, j, west, east, westLongitudeIndex, eastLongitudeIndex);
}

void RegularGaussian::generateGrid1D(vector<Point>& llgrid, const Grid& grid)  const {
    if(grid.rotated()) {
        Rotation rot(grid.latPole(),grid.lonPole());
        generateRotatedGrid(llgrid,rot);
    } else
        generateGrid(llgrid);
}

void RegularGaussian::print(ostream& out) const {
    out << "RegularGaussian-> " ;
    Gaussian::print(out);
    Regular::print(out);
}

bool RegularGaussian::sameAs(const RegularGaussian& other) const {
    return gaussianNumber_ == other.gaussianNumber_;
}
