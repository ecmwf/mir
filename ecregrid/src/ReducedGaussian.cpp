/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "ReducedGaussian.h"

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

ReducedGaussian::ReducedGaussian(double north,double west,double south,double east, int n, long* gridDef, size_t gridSize):
    Gaussian(north, west, south, east, n), Reduced(this,gridDef,gridSize) {
}

ReducedGaussian::ReducedGaussian(double north, double west, double south, double east, int n):
    Gaussian(north, west, south, east, n), Reduced(this) {
    if (!isValidGaussianNumber(n))
        throw WrongGaussianNumber(n);
}

ReducedGaussian::ReducedGaussian(const Area& other, int n, long* gridDef, size_t gridSize):
    Gaussian(other,n), Reduced(this,gridDef,gridSize) {
}

ReducedGaussian::ReducedGaussian(const Area& other, int n):
    Gaussian(other,n), Reduced(this) {
    if (!isValidGaussianNumber(n))
        throw WrongGaussianNumber(n);
}

ReducedGaussian::ReducedGaussian(int n):
    Gaussian(n), Reduced(this) {
    if (!isValidGaussianNumber(n))
        throw WrongGaussianNumber(n);
}

ReducedGaussian::ReducedGaussian(bool global, int n):
    Gaussian(n), Reduced(this) {
    if (!isValidGaussianNumber(n))
        throw WrongGaussianNumber(n);
    setGlobalArea();
}

ReducedGaussian::~ReducedGaussian() {
}

bool ReducedGaussian::isValidGaussianNumber(int n) const {
    return (n == 32)   ||
           (n == 48)   ||
           (n == 80)   ||
           (n == 128)  ||
           (n == 160)  ||
           (n == 200)  ||
           (n == 256)  ||
           (n == 320)  ||
           (n == 400)  ||
           (n == 512)  ||
           (n == 640)  ||
           (n == 1024) ||
           (n == 2000) ||
           (n == 4000);
}

string ReducedGaussian::predefinedLsmFileName() const {
    stringstream lsmFile;
    lsmFile << "ReducedGaussian_" << gaussianNumber_;

    return lsmFile.str();
}

void ReducedGaussian::findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex ) const {
    Reduced::findWestAndEastIndex(ctx, j, west, east, westLongitudeIndex,eastLongitudeIndex);
}

void ReducedGaussian::accumulatedDataWE(const double* data, unsigned long dataLength,vector<double>& newData) const {
    return Reduced::accumulatedDataWestEast(data,dataLength,newData);
}

void ReducedGaussian::reOrderNewData(const vector<double>& data_in, vector<double>& data_out, int scMode) const {
    Reduced::reOrderData(data_in, data_out, scMode);
}

void ReducedGaussian::getGridAsLatLonList(double* lats, double* lons, long* length)  const {
    generateGridAsLatLonList(lats,lons,length);
}

void ReducedGaussian::generateGrid1D(vector<Point>& llgrid)  const {
    generateGrid(llgrid);
}

void ReducedGaussian::generateGrid1D(vector<Point>& llgrid, const Grid& grid)  const {
    if(grid.rotated()) {
        Rotation rot(grid.latPole(),grid.lonPole());
        generateRotatedGrid(llgrid,rot);
    } else
        generateGrid(llgrid);
}

long  ReducedGaussian::getLatitudeOffset(double lat, long& current) const {
    return Reduced::getLatitudeOffset(lat,current);
}

int  ReducedGaussian::getLatitudeOffset(double lat) const {
    return Reduced::getLatitudeOffset(lat);
}

void  ReducedGaussian::getOffsets(vector<int>& offsets) const {
    return Reduced::getOffsets(offsets);
}

size_t  ReducedGaussian::getGridDefinition(vector<long>& v) const {
    return Reduced::getGridDefinition(v);
}

bool ReducedGaussian::sameAs(const ReducedGaussian& other) const {
    return gaussianNumber() == other.gaussianNumber();
    /*
    if(gaussianNumber() != other.gaussianNumber())
    	return false;

    vector<long> thisGridDef;
    size_t thisSize = getGridDefinition(thisGridDef);
    vector<long> otherGridDef;
    size_t otherSize = other.getGridDefinition(otherGridDef);

    //	ASSERT(thisSize == otherSize);

    if(thisSize != otherSize)
    		return false;

    for(size_t i = 0; i < thisSize; i++)
    	if(thisGridDef[i] != otherGridDef[i])
    		return false;
    return true;
    */
}

long ReducedGaussian::getIndex(int i, int j) const {
    return Reduced::getIndex(i,j);
}

void  ReducedGaussian::addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const {
    return Reduced::addWeightsAlongLatitude(weights,weightY,i);
}

unsigned long ReducedGaussian::calculatedNumberOfPoints() const {
    return Reduced::calculatedNumberOfPoints();
}

int ReducedGaussian::lengthOfFirstLatitude() const {
    return Reduced::lengthOfFirstLat();
}

int ReducedGaussian::lengthOfLastLatitude() const {
    return Reduced::lengthOfLastLat();
}

int ReducedGaussian::westEastNumberOfPoints() const {
    throw NotImplementedFeature("ReducedGaussian - westEastNumberOfPoints()");
}

Grid* ReducedGaussian::newGrid(const Grid& input) const {
    Area area = areaFix(input);
    return new ReducedGaussian(area,gaussianNumber_);
}

Grid* ReducedGaussian::newGrid(const Area& area) const {
    return new ReducedGaussian(area,gaussianNumber_);
}

Grid* ReducedGaussian::getGlobalGrid() const {
    return new ReducedGaussian(setGlobalArea(),gaussianNumber_);
}

double ReducedGaussian::conserving(GridContext* ctx,const Point& where, const vector<double>& data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const {
    return Reduced::conserving(ctx,where,data,scMode,missingValue,outNSincrement,outWEincrement);
}

void ReducedGaussian::cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const {
    Reduced::cellsAreas(areas,areasSize);
}

double ReducedGaussian::fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double ns, double we)  const {
    return Reduced::fluxConserving(ctx,where,areas,inputCellSize,data,scMode,missingValue,ns,we);
}

double ReducedGaussian::averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double we) const {
    return Reduced::averageWeighted(ctx,where,weights,data,scMode,missingValue,outLats,we);
}

double ReducedGaussian::averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double we) const {
    return Reduced::averageWeightedLsm(ctx,where,weights,data,dataLsmIn,dataLsmOut,scMode,missingValue,outLats,we);
}

void ReducedGaussian::nearestsByIndex(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const {
    nearestIndexed(where,nearest,data,scMode,missingValue,howMany);
}

void ReducedGaussian::nearestPoints(GridContext* ctx,const Point& where,vector<FieldPoint>& result,const vector<double>& data,int scMode, int npts) const {
    nearestPts(ctx,where,result,data,scMode,npts);
}

void ReducedGaussian::nearest4pts(GridContext* ctx, const Point& where, vector<Point>& result) const {
    nearest4(ctx,where,result);
}

void ReducedGaussian::print(ostream& out) const {
    out << "ReducedGaussian-> " ;
    Gaussian::print(out);
    Reduced::print(out);
}

