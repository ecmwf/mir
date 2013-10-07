/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "PseudoRegularGaussian.h"

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

PseudoRegularGaussian::PseudoRegularGaussian(double north, double west, double south, double east, int n, int we):
	Gaussian(north, west, south, east, n), 
    Regular(this),
    westEastNumberOfPoints_(we)
{
	int calculated = Gaussian::westEastNumberOfPoints();
	if(!we){
		westEastNumberOfPoints_ = calculated;
		if(DEBUG)
			cout << "PseudoRegularGaussian::PseudoRegularGaussian we = 0 => calculated: " << westEastNumberOfPoints_ << endl;
	}

	if(westEastNumberOfPoints_ != calculated)
		cout << "PseudoRegularGaussian::PseudoRegularGaussian WARNING pseudo gaussian grid: " << n << endl;
	
}

PseudoRegularGaussian::PseudoRegularGaussian(int n, int we):
	Gaussian(n), 
    Regular(this),
    westEastNumberOfPoints_(we) 
{
	int calculated = Gaussian::westEastNumberOfPoints();
	if(!we)
		westEastNumberOfPoints_ = calculated;
		if(DEBUG){
			cout << "PseudoRegularGaussian::PseudoRegularGaussian we = 0 => calculated: " << westEastNumberOfPoints_ << endl;
	}

	if(westEastNumberOfPoints_ != calculated)
		cout << "PseudoRegularGaussian::PseudoRegularGaussian WARNING pseudo gaussian grid: " << n << endl;
}

PseudoRegularGaussian::PseudoRegularGaussian(const Area& other, int n, int we):
	Gaussian(other, n), 
    Regular(this),
    westEastNumberOfPoints_(we) 
{
	int calculated = Gaussian::westEastNumberOfPoints();
	if(!we){
		westEastNumberOfPoints_ = calculated;
		if(DEBUG)
			cout << "PseudoRegularGaussian::PseudoRegularGaussian we = 0 => calculated: " << westEastNumberOfPoints_ << endl;
	}

	if(westEastNumberOfPoints_ != calculated)
		cout << "PseudoRegularGaussian::PseudoRegularGaussian WARNING pseudo gaussian grid: " << n << endl;
}

PseudoRegularGaussian::~PseudoRegularGaussian()
{
}

void PseudoRegularGaussian::findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex ) const
{
	Regular::findWestAndEastIndex(ctx, j, west, east, westLongitudeIndex, eastLongitudeIndex);	
}

void  PseudoRegularGaussian::addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const
{
	return Regular::addWeightsAlongLatitude(weights,weightY,i);
}

long PseudoRegularGaussian::getIndex(int i, int j) const
{
	return Regular::getIndex(i,j);
}

string PseudoRegularGaussian::predefinedLsmFileName() const 
{
	stringstream lsmFile;
	lsmFile << "PseudoRegularGaussian_" << gaussianNumber_;

	return lsmFile.str();
}

void PseudoRegularGaussian::accumulatedDataWE(const double* data, unsigned long dataLength, vector<double>& newData) const
{
	return Regular::accumulatedDataWestEast(data,dataLength,newData);
}

void PseudoRegularGaussian::reOrderNewData(const vector<double>& data_in, vector<double>& data_out, int scMode) const
{
	Regular::reOrderData(data_in, data_out, scMode);
}

Grid* PseudoRegularGaussian::newGrid(const Grid& input) const
{
     Area area = areaFix(input);
	    return new PseudoRegularGaussian(area,gaussianNumber_);
}

Grid* PseudoRegularGaussian::newGrid(const Area& area) const
{
	return new PseudoRegularGaussian(area,gaussianNumber_);
}

Grid* PseudoRegularGaussian::getGlobalGrid() const
{
	    return new PseudoRegularGaussian(setGlobalArea(),gaussianNumber_);
}

size_t  PseudoRegularGaussian::getGridDefinition(vector<long>& v) const
{
	return Regular::getGridDefinition(v);
}

int  PseudoRegularGaussian::getLatitudeOffset(double lat) const
{
	return Regular::getLatitudeOffset(lat);
}

long  PseudoRegularGaussian::getLatitudeOffset(double lat, long& current) const
{
	return Regular::getLatitudeOffset(lat,current);
}

void  PseudoRegularGaussian::getOffsets(vector<int>& offsets) const
{
	return Regular::getOffsets(offsets);
}

unsigned long PseudoRegularGaussian::calculatedNumberOfPoints() const
{
	if(DEBUG){
		cout << "PseudoRegularGaussian::calculatedNumberOfPoints =>northSouthNumberOfPoints: " << northSouthNumberOfPoints() << endl;
		cout << "PseudoRegularGaussian::calculatedNumberOfPoints => westEastNumberOfPoints: " << westEastNumberOfPoints_ << endl;
	}
	return westEastNumberOfPoints_ * northSouthNumberOfPoints();
}

void PseudoRegularGaussian::nearestsByIndex(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const
{
	nearestIndexed(where,nearest,data,scMode,missingValue,howMany);
}

double PseudoRegularGaussian::conserving(GridContext* ctx,const Point& where, const vector<double>& data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const
{
	return Regular::conserving(ctx,where,data,scMode,missingValue,outNSincrement,outWEincrement);
}

void PseudoRegularGaussian::cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const 
{
	Regular::cellsAreas(areas,areasSize);
}

double PseudoRegularGaussian::fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double ns, double we)  const
{
	return Regular::fluxConserving(ctx,where,areas,inputCellSize,data,scMode,missingValue,ns,we);
}

double PseudoRegularGaussian::averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double we) const
{
	return Regular::averageWeighted(ctx,where,weights,data,scMode,missingValue,outLats,we);
}

double PseudoRegularGaussian::averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double we) const
{
	return Regular::averageWeightedLsm(ctx,where,weights,data,dataLsmIn,dataLsmOut,scMode,missingValue,outLats,we);
}

void PseudoRegularGaussian::nearestPoints(GridContext* ctx,const Point& where,vector<FieldPoint>& result, const vector<double>& data, int scMode, int npts)  const
{
	nearestPts(ctx,where,result,data,scMode,npts);
}

void PseudoRegularGaussian::nearest4pts(GridContext* ctx, const Point& where, vector<Point>& result) const
{
    nearest4(ctx,where,result);
}

void PseudoRegularGaussian::getGridAsLatLonList(double* lats, double* lons, long* length)  const 
{
    generateGridAsLatLonList(lats,lons,length);
}

void PseudoRegularGaussian::generateGrid1D(vector<Point>& llgrid)  const 
{
    generateGrid(llgrid);
}

void PseudoRegularGaussian::generateGrid1D(vector<Point>& llgrid, const Grid& grid)  const 
{
	if(grid.rotated()){
		Rotation rot(grid.latPole(),grid.lonPole());
    	generateRotatedGrid(llgrid,rot);
	}
	else
    	generateGrid(llgrid);
}

bool PseudoRegularGaussian::sameAs(const PseudoRegularGaussian& other) const  
{ 
	return gaussianNumber_ == other.gaussianNumber_ && westEastNumberOfPoints_ == other.westEastNumberOfPoints_; 
}

Area PseudoRegularGaussian::setGlobalArea(double west) const
{
	if(west == -180.0)
		return Area(latitudes_[0], -180.0, latitudes_[gaussianNumber_*2-1],(180.0 - 360./westEastNumberOfPoints_));

	return Area(latitudes_[0], 0, latitudes_[gaussianNumber_*2-1],(westEastNumberOfPoints_ - 1) * (360.0 / westEastNumberOfPoints_));

}

Area PseudoRegularGaussian::setGlobalArea() const
{
	return Area(latitudes_[0], 0, latitudes_[gaussianNumber_*2-1],(westEastNumberOfPoints_ - 1) * (360.0 / westEastNumberOfPoints_));
}

/*
bool PseudoRegularGaussian::isGlobalWestEast() const
{
	return (east() - west() + 360.0 / westEastNumberOfPoints_ + AREA_FACTOR) >= 360.0;
}
*/

Area PseudoRegularGaussian::adjustArea(const Area& area, bool globalWE, bool globalNS) const
{
	double west = area.west();
	double east = area.east();

	if(DEBUG){
		cout << "PseudoRegularGaussian::adjustArea west: " << west << " east " << east << endl;
		cout << "PseudoRegularGaussian::adjustArea is global west-east: " << globalWE << " is global north-south " << globalNS << endl;
	}

	if(globalWE)
		setGlobalWestEast(west,east,360.0/westEastNumberOfPoints_);
	else
		adjustAreaWestEast(west,east,90.0/gaussianNumber_);

    double north = 0, south = 0;

    if(globalNS){
        setGlobalNorthSouth(north,south);
	}
	else{
		north = latitudes_[northIndex(area.north())];
		south = latitudes_[southIndex(area.south())];
	}

	return Area(north,west,south,east);
}

void PseudoRegularGaussian::print(ostream& out) const
{
	out << "PseudoRegularGaussian-> " ;
	Gaussian::print(out);
	Regular::print(out); 
	out << ", West-East number of points=[" << westEastNumberOfPoints_ << "]";
}

