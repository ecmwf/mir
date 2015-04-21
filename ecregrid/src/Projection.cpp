/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Projection.h"

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

Projection::Projection() :
    Grid() {
}

Projection::Projection(const vector<Point>& llgrid, double north, double west, double south, double east) :
    Grid(north, west, south, east), points_(llgrid) {
}

Projection::~Projection() {
}

void Projection::findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex ) const {
    throw NotImplementedFeature("Projection::findWestAndEastIndex");
}

Grid* Projection::getGlobalGrid() const {
    throw NotImplementedFeature("Projection::getGlobalGrid");
}

void Projection::nearestPoints(GridContext* ctx,const Point& where,vector<FieldPoint>& result, const vector<double>& data, int scMode, int npts)  const {
    throw NotImplementedFeature("Projection::nearestPoints");
}

void Projection::nearest4pts(GridContext* ctx, const Point& where, vector<Point>& result) const {
    throw NotImplementedFeature("Projection::nearest4pts");
//    nearest4(ctx,where,result);
}

double Projection::conserving(GridContext* ctx,const Point& where, const vector<double>& data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const {
    throw NotImplementedFeature("Projection::conserving");
}

void Projection::cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const {
    throw NotImplementedFeature("Projection::cellsAreas");
}
/*
void ListOfPoints::global()
{
	north_ = 0;
	west_  = 0;
	south_ = 0;
	east_  = 0;
}
*/

double Projection::fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const {
    throw NotImplementedFeature("Projection::fluxConserving");
    return 0;
}

double Projection::averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double outWEincrement) const {
    throw NotImplementedFeature("Projection::nearestPointsForAverage");
    return 0;
}

double Projection::averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double we) const {
    throw NotImplementedFeature("Projection::averageWeightedLsm");
    return 0;
}

void Projection::nearestsByIndex(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const {
    throw NotImplementedFeature("Projection::nearestsByIndex");
}

unsigned long Projection::calculatedNumberOfPoints() const {
    return (unsigned long)points_.size();
}

size_t  Projection::getGridDefinition(vector<long>& v) const {
    throw NotImplementedFeature("Projection::getGridDefinition");
}

int  Projection::getLatitudeOffset(double lat) const {
    throw NotImplementedFeature("Projection::getLatitudeOffset");
}

long  Projection::getLatitudeOffset(double lat, long& current) const {
    throw NotImplementedFeature("Projection::getLatitudeOffset");
}

void  Projection::getOffsets(vector<int>& offsets) const {
    throw NotImplementedFeature("Projection::getOffsets");
}


void Projection::reOrderNewData(const vector<double>& data_in, vector<double>& data_out, int scMode) const {
    throw NotImplementedFeature("Projection::reOrderNewData");
}

void Projection::accumulatedDataWE(const double* data, unsigned long dataLength, vector<double>& newData) const {
    throw NotImplementedFeature("Projection::accumulatedDataWE");
}

void  Projection::addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const {
    throw NotImplementedFeature("Projection::addWeightsAlongLatitude");
//	return Regular::addWeightsAlongLatitude(weights,weightY,i);
}

size_t Projection::loadGridSpec(vector<long>& rgSpec) const {
    throw NotImplementedFeature("Projection::loadGridSpec");
    return 0;
}

void Projection::weightsY(vector<double>& w) const {
    throw NotImplementedFeature("Projection::weightsY");
}

int  Projection::truncate(int truncation) const {
    throw NotImplementedFeature("Projection::truncate");
    return 0;
}

int Projection::matchGaussian() const {
    throw NotImplementedFeature("Projection::matchGaussian");
    return 0;
}

long Projection::getIndex(int i, int j) const {
    throw NotImplementedFeature("Projection::getIndex");
    return 0;
//	return Regular::getIndex(i,j);
}

void Projection::getGridAsLatLonList(double* lats, double* lons, long* length)  const {
    throw NotImplementedFeature("Projection::getGridAsLatLonList");
//    generateGridAsLatLonList(lats,lons,length);
}

void Projection::generateGrid1D(vector<Point>& llgrid) const {
    llgrid.clear();
    llgrid.assign( points_.begin(), points_.end() );
}

void Projection::generateGrid1D(vector<Point>& llgrid, const Grid& grid)  const {
    if(grid.rotated()) {
        throw NotImplementedFeature("Projection::generateGrid1D rotation");
    } else
        generateGrid1D(llgrid);
}

GridContext* Projection::getGridContext() const {
    throw NotImplementedFeature("Projection::getGridContext");
}

vector<double> Projection::latitudes() const {
    throw NotImplementedFeature("Projection::latitudes");
}

void Projection::latitudes(vector<double>& lats) const {
    throw NotImplementedFeature("Projection::latitudes");
}

void Projection::northHemisphereLatitudes(vector<double>& lats) const {
    throw NotImplementedFeature("Projection::northHemisphereLatitudes");
}

int Projection::poleEquatorNumberOfPoints() const {
    throw NotImplementedFeature("Projection::poleEquatorNumberOfPoints");
    return 0;
}


Area Projection::fitToGlobalArea(double west) const {
    throw NotImplementedFeature("Projection::fitToGlobalArea");
}

Area Projection::setGlobalArea() const {
    throw NotImplementedFeature("Projection::setGlobalArea");
}

Area Projection::setGlobalArea(double west) const {
    throw NotImplementedFeature("Projection::setGlobalArea");
}

string Projection::coeffInfo() const {
    throw NotImplementedFeature("Projection::coeffInfo");
}

void Projection::adjustAreaWestEastMars( double& west, double& east, double increment) const {
    throw NotImplementedFeature("Projection::adjustAreaWestEastMars");
}

int Projection::northIndex(double north) const {
    throw NotImplementedFeature("Projection::northIndex");
}

int Projection::southIndex(double south) const {
    throw NotImplementedFeature("Projection::southIndex");
}

void Projection::setGlobalNorthSouth(double& north, double& south) const {
    throw NotImplementedFeature("Projection::setGlobalNorthSouth");
}

bool Projection::isGlobalNorthSouth() const {
    throw NotImplementedFeature("Projection::isGlobalNorthSouth");
}

Area Projection::adjustArea( const Area& area, bool globalWE, bool globalNS) const {
    throw NotImplementedFeature("Projection::adjustArea");
}

void Projection::print(ostream& out) const {
    out << "Projection{ " ;
    Grid::print(out);
}
