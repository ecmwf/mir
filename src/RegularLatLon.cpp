/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "RegularLatLon.h"

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

RegularLatLon::RegularLatLon(double ns, double we):
    LatLon(ns,we), Regular(this)
{
}

RegularLatLon::RegularLatLon(double north, double west, double south, double east, double ns, double we): 
	LatLon(north, west, south, east, ns, we), Regular(this)
{
}

RegularLatLon::RegularLatLon(const Area& other, double ns, double we): 
	LatLon(other, ns, we), Regular(this) 
{
}

// with Number of Points and increments
RegularLatLon::RegularLatLon(double north, double west, double south, double east, double ns, double we, long nptsNS, long nptsWE): 
	LatLon(north,west,south,east,ns,we), Regular(this,nptsNS,nptsWE)
{
	ASSERT(nptsNS);
    double newns = calculateNortSouthIncrement(north,south,nptsNS);
    if(fabs(ns - newns) > AREA_FACTOR){
        cout << "!!!!!!!!!!! RegularLatLon::RegularLatLon calculated North-South increment: " << newns << " different from provided " << ns << endl;
    }

    ASSERT(nptsWE);
    double newwe = calculateWestEastIncrement(east,west,we,nptsWE);
    if(fabs(we - newwe) > AREA_FACTOR){
        cout << "!!!!!!!!!!! RegularLatLon::RegularLatLon calculated West-East increment: " << newwe << " different from provided " << we << endl;
    }
}

RegularLatLon::RegularLatLon(long nptsNS, long nptsWE): 
	LatLon(calculateNortSouthIncrement(90.0,-90.0,nptsNS),calculateWestEastIncrement(360,0,true,nptsWE)), Regular(this,nptsNS,nptsWE) 
{
	ASSERT(nptsNS);
    ASSERT(nptsWE);
}

RegularLatLon::RegularLatLon(const Area& area, double ns, double we, long nptsNS, long nptsWE): 
	LatLon(area,ns,we), Regular(this,nptsNS,nptsWE) 
{
	ASSERT(nptsNS);
    double newns = calculateNortSouthIncrement(area.north(),area.south(),nptsNS);
    if(fabs(ns - newns) > AREA_FACTOR){
        cout << "!!!!!!!!!!! RegularLatLon::RegularLatLon calculated North-South increment: " << newns << " different from provided " << ns << endl;
    }

    ASSERT(nptsWE);
    double newwe = calculateWestEastIncrement(area.east(),area.west(),we,nptsWE);
    if(fabs(we - newwe) > AREA_FACTOR){
        cout << "!!!!!!!!!!! RegularLatLon::RegularLatLon calculated West-East increment: " << newwe << " different from provided " << we << endl;
    }
}

// only Number of Points
RegularLatLon::RegularLatLon(const Area& area, long nptsNS, long nptsWE,bool globalWE)
      : LatLon(area,calculateNortSouthIncrement(area.north(), area.south(),nptsNS),calculateWestEastIncrement(area.east(),area.west(),globalWE,nptsWE)), Regular(this,nptsNS,nptsWE)
{
}

RegularLatLon::RegularLatLon(double north, double west, double south, double east, long nptsNS, long nptsWE, bool globalWE)
          : LatLon(north,west,south,east,calculateNortSouthIncrement(north,south,nptsNS),calculateWestEastIncrement(east,west,globalWE,nptsWE)), Regular(this,nptsNS,nptsWE)
{
}

RegularLatLon::~RegularLatLon()
{
}

string RegularLatLon::predefinedLsmFileName() const 
{
    return LatLon::constructLsmFilename("RegularLatLon_");
}

void RegularLatLon::accumulatedDataWE(const double* data, unsigned long dataLength, vector<double>& newData) const
{
	return Regular::accumulatedDataWestEast(data,dataLength,newData);
}

void RegularLatLon::reOrderNewData(const vector<double>& data_in, vector<double>& data_out, int scMode) const
{
	Regular::reOrderData(data_in, data_out, scMode);
}

Grid* RegularLatLon::newGrid(const Grid& other) const
{
     Area areaNew = areaFix(other);
//	 cout << "RegularLatLon::newGrid areaFix " << area << endl;
	    return new RegularLatLon(areaNew, ns_,we_);
}

Grid* RegularLatLon::newGrid(const Area& area) const
{
	    return new RegularLatLon(area, ns_,we_);
}

Grid* RegularLatLon::getGlobalGrid() const
{
	    return new RegularLatLon(setGlobalArea(),ns_,we_);
}

size_t  RegularLatLon::getGridDefinition(vector<long>& v) const
{
 	return Regular::getGridDefinition(v);
}

void RegularLatLon::findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex ) const
{
	Regular::findWestAndEastIndex(ctx, j, west, east, westLongitudeIndex,eastLongitudeIndex);	
}

int  RegularLatLon::getLatitudeOffset(double lat) const
{
	return Regular::getLatitudeOffset(lat);
}

long  RegularLatLon::getLatitudeOffset(double lat, long& current) const
{
	return Regular::getLatitudeOffset(lat,current);
}

void  RegularLatLon::getOffsets(vector<int>& offsets) const
{
	return Regular::getOffsets(offsets);
}

long RegularLatLon::getIndex(int i, int j) const
{
	return Regular::getIndex(i,j);
}

void  RegularLatLon::addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const
{
	return Regular::addWeightsAlongLatitude(weights,weightY,i);
}

unsigned long RegularLatLon::calculatedNumberOfPoints() const
{
//	return  northSouthNumberOfPoints() * westEastNumberOfPoints() ;
	return  calculatedNumberOfPts() ;
}

void RegularLatLon::nearestsByIndex(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const
{
	nearestIndexed(where,nearest,data,scMode,missingValue,howMany);
}

void RegularLatLon::generateGrid1D(vector<Point>& llgrid)  const 
{
    generateGrid(llgrid);
}

void RegularLatLon::getGridAsLatLonList(double* lats, double* lons, long* length)  const 
{
    generateGridAsLatLonList(lats,lons,length);
}

void RegularLatLon::generateGrid1D(vector<Point>& llgrid, const Grid& grid)  const 
{
	if(grid.rotated()){
		Rotation rot(grid.latPole(),grid.lonPole());
    	generateRotatedGrid(llgrid,rot);
	}
	else
    	generateGrid(llgrid);
}


void RegularLatLon::nearestPoints(GridContext* ctx,const Point& where,vector<FieldPoint>& result, const vector<double>& data, int scMode, int npts)  const
{
	nearestPts(ctx,where,result,data,scMode,npts);
}

void RegularLatLon::nearest4pts(GridContext* ctx, const Point& where, vector<Point>& result) const
{
    nearest4(ctx,where,result);
}

double RegularLatLon::conserving(GridContext* ctx,const Point& where, const vector<double>& data, int scMode, double missingValue, double outNSincrement, double outWEincrement) const
{
	return Regular::conserving(ctx,where,data,scMode,missingValue,outNSincrement,outWEincrement);
}

void RegularLatLon::cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const 
{
	Regular::cellsAreas(areas,areasSize);
}

double RegularLatLon::fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double ns, double we)  const
{
	return Regular::fluxConserving(ctx,where,areas,inputCellSize,data,scMode,missingValue,ns,we);
}

double RegularLatLon::averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double we) const
{
	return Regular::averageWeighted(ctx,where,weights,data,scMode,missingValue,outLats,we);
}

double RegularLatLon::averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double we) const
{
	return Regular::averageWeightedLsm(ctx,where,weights,data,dataLsmIn,dataLsmOut,scMode,missingValue,outLats,we);
}

void RegularLatLon::print(ostream& out) const
{
	out << "RegularLatLon-> " ; LatLon::print(out); Regular::print(out); 
}

bool RegularLatLon::sameAs(const  RegularLatLon& other) const  
{
	return ns_ == other.ns_ && we_ == other.we_; 
}

