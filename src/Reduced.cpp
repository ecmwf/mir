/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Reduced.h"


#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef ReducedDataHelper_H
#include "ReducedDataHelper.h"
#endif

#ifndef Rotation_H
#include "Rotation.h"
#endif

#ifndef GridContext_H
#include "GridContext.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef Linear_H
#include "Linear.h"
#endif

#ifndef Area_H
#include "Area.h"
#endif

#include <algorithm>
#include <iterator>
#include <climits>
#include <cfloat>


typedef ReducedDataHelper<double> DoubleHelper;

Reduced::Reduced(const Grid* grid):
	GridType(grid->isGlobalWestEast(),grid->isGlobalNorthSouth()), northLatitudeIndex_(grid->northIndex(grid->north())), west_(grid->west()), east_(grid->east())
{
	grid->latitudes(latitudes_);
	grid->loadGridSpec(gridDefinition_);

	if(DEBUG){
		cout << "Reduced::Reduced north: " << grid->north() << " south: " << grid->south() << " west: " << west_ << " east: " << east_ << endl;
		cout << "Reduced::Reduced globalWestEast_: " << globalWestEast_ << " globalNorthSouth_: " << globalNorthSouth_ << endl;
	}

	gridLatNumber_  = latitudes_.size();
	indexOfLastLat_ = gridLatNumber_ - 1;

	mappedLatitudes(grid,allLons_,offsets_);
	calculatedNumberOfPoints_ = calculateNpts();
	if(DEBUG)
		cout << "Reduced::Reduced calculatedNumberOfPoints: " << calculatedNumberOfPoints_ << endl;
	delta_ = 180.0 / gridLatNumber_;

//	printMappedLatitudes(cout);
}

Reduced::Reduced(const Grid* grid, const long* gridDefinition, size_t gridLatNumber):
	GridType(grid->isGlobalWestEast(),grid->isGlobalNorthSouth()), gridLatNumber_(gridLatNumber),indexOfLastLat_(gridLatNumber - 1),northLatitudeIndex_(grid->northIndex(grid->north())), west_(grid->west()), east_(grid->east())
{
	grid->latitudes(latitudes_);
	gridDefinition_.assign(gridDefinition, gridDefinition + gridLatNumber);

	mappedLatitudes(grid,allLons_,offsets_);
	calculatedNumberOfPoints_ = calculateNpts();
	delta_ = 180.0 / gridLatNumber_;
//	printMappedLatitudes(cout);
}

Reduced::~Reduced()
{
//	delete [] gridDefinition_;
}

long Reduced::operator()(int i,int j) const 
{  
	long t = offsets_[j] + i;
	ASSERT(t >= 0);

	if(t >= (long)calculatedNumberOfPoints_){
		throw OutOfRange(t,calculatedNumberOfPoints_);
	}

	return t;
}

long Reduced::getIndex(int i,int j) const 
{  
	long t = offsets_[j] + i;
	ASSERT(t >= 0);

	if(t >= (long)calculatedNumberOfPoints_){
		throw OutOfRange(t,calculatedNumberOfPoints_);
	}

	return t;
}


void Reduced::accumulatedDataWestEast(const double* data, unsigned long dataLength, vector<double>& newData) const
{
	ASSERT(calculatedNumberOfPoints() == dataLength);
	newData.clear();
	newData.reserve(dataLength);

	long count   = 0;
	for(size_t j = 0; j < gridLatNumber_; j++) {
		long lonNumber = gridDefinition_[j];	
		double temp = 0;
		for(int i = 0 ; i <  lonNumber; ++i) {
			temp += data[count++];
			newData.push_back(temp);
		}
	}
}

void Reduced::addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const
{
	long size = gridDefinition_[i];
	double weight = weightY / size;

	for (long j = 0; j < size; j++){
		weights.push_back(weight);
	}
}


void Reduced::nearestIndexed(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const
{
	nearest.clear();
	DoubleHelper  values(data,scMode,missingValue,offsets_,gridLatNumber_);

	int iInd   = where.iIndex();
	int jInd   = where.jIndex();

	int lastJIndex = gridLatNumber_ -1;
	int lastIIndex = allLons_[jInd].size() - 1;

	double wlat = where.latitude();
	double wlon = where.longitude();

	if(same(wlon,360.0))
		wlon = 0.0;

// ssp It assumes the grid is global
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
	double west, east;
	int wNorth, eNorth, wSouth, eSouth;
	int iupclose,idownclose;
	double lonup,londown;
	double value;

// north
	if(jup != -1 && allLons_[jup].size()) {
		findWestEastNeighbours(wlon,iInd,allLons_[jup],west,east,wNorth,eNorth);
		if(getenv("ECREGRID_LINEAR_DERIVATIVES")){
			Linear linear;
			Point p(latitudes_[jup],wlon);
			vector<FieldPoint> temp;
			temp.push_back(FieldPoint(latitudes_[jup],west,wNorth,jup,values(wNorth,jup)));
			temp.push_back(FieldPoint(latitudes_[jup],east,eNorth,jup,values(eNorth,jup)));
			value = linear.interpolatedValue(p,temp);
			lonup = wlon;
			iupclose = 9999;
		}
		else{
			if(fabs(west - wlon) >= fabs(wlon - east)){
				lonup = east;
				iupclose = eNorth;
				value = values(eNorth,jup);
			}
			else{
				lonup = west;
				iupclose = wNorth;
				value = values(wNorth,jup);
			}
		}
			
	}
	else{
		lonup    = wlon;
		iupclose = 9999;
		value    = missingValue;
	}
	// 0
	nearest.push_back( FieldPoint(latitudes_[jup],lonup,iupclose,jup,value));
//-------------------------------------------------------------------------------
	// 1
    value = values(ileft,jInd);
	nearest.push_back( FieldPoint(wlat, allLons_[jInd][ileft],ileft,jInd,value));
//-------------------------------------------------------------------------------
// south
	if(jdown != -1 && allLons_[jdown].size()) {
		findWestEastNeighbours(wlon,iInd,allLons_[jdown],west,east,wSouth,eSouth);
		if(getenv("ECREGRID_LINEAR_DERIV")){
			Linear linear;
			Point p(latitudes_[jdown],wlon);
			vector<FieldPoint> temp;
			temp.push_back(FieldPoint(latitudes_[jdown],west,wSouth,jdown,values(wSouth,jdown)));
			temp.push_back(FieldPoint(latitudes_[jdown],east,eSouth,jdown,values(eSouth,jdown)));
			value = linear.interpolatedValue(p,temp);
			londown = wlon;
			idownclose = 9999;
		}
		else{
			if(fabs(west - wlon) >= fabs(wlon - east)){
				londown = east;
				idownclose = eSouth;
				value = values(eSouth,jdown);
			}
			else{
				londown = west;
				idownclose = wSouth;
				value = values(wSouth,jdown);
			}
		}
	}
	else{
		londown    = wlon;
		idownclose = 9999;
		value      = missingValue;
	}
	// 2
	nearest.push_back( FieldPoint(latitudes_[jdown],londown,idownclose,jdown,value));
//-----------------------------------------------------------------------------
	// 3
    value = values(iright,jInd);
	nearest.push_back( FieldPoint(wlat,allLons_[jInd][iright],iright,jInd,value));
}

void Reduced::reOrderData(const vector<double>& data_in, vector<double>& data_out, int scMode) const
{
	vector<long>::const_iterator iter, start = offsets_.begin(), end = offsets_.end();

    const size_t dataLength = data_in.size();
    data_out.resize(dataLength);

	if(scMode == 2) {
		long count = 0;
		int rowEnd = dataLength;	

		iter = end;
		do{
			iter--;
			int currOffset = *iter;	
			for(int i = currOffset ; i <  rowEnd; ++i) {
				data_out[count++] = data_in[i];
			}
			rowEnd = *iter;
		}
		while(iter != start);

		if(DEBUG){
			cout << "================================================================== " << endl;
			cout << "====              Output Data Re Ordered                  ==== " << endl;
			cout << "Reduced::reOrderData Output Scanning Mode is West-East South-North " << endl;
			cout << "================================================================== " << endl;
		}

        return;
	}

	if(scMode == 3) {
		long count = 0;
		int currEnd = 0;	

		for( ; start != end; start++){
			int currOffset = *start;	
			iter = start;
			iter++;
			if( iter != end)
				currEnd = *iter;	
			else
				currEnd = dataLength - 1;	
			for(int i = currEnd ; i > currOffset ; --i) {
				data_out[count++] = data_in[i];
			}
		}
		if(DEBUG){
			cout << "================================================================== " << endl;
			cout << "====              Output Data Re Ordered                  ==== " << endl;
			cout << "Reduced::reOrderData Output Scanning Mode is East-West North-South " << endl;
			cout << "================================================================== " << endl;
		}
		return;
	}

	if(scMode == 4) {
		long count = 0;
		int currStart = dataLength - 1;	

		iter = end;
		do{
			iter--;
			int currOffset = *iter;	
			for(int i = currStart ; i > currOffset; --i) {
				data_out[count++] = data_in[i];
			}
		}
		while(iter != start);
		if(DEBUG){
			cout << "================================================================== " << endl;
			cout << "====              Output Data Re Ordered                  ==== " << endl;
			cout << "Reduced::reOrderData Output Scanning Mode is East-West South-North " << endl;
			cout << "================================================================== " << endl;
		}

		return;
	}

	throw WrongValue("Reduced::reOrderData  Scanning Mode",scMode);	
}

size_t  Reduced::getGridDefinition(vector<long>& grDef) const
{
	grDef.assign(gridDefinition_.begin(),gridDefinition_.end());
	return  gridLatNumber_;
}

void Reduced::mappedLatitudes(const Grid* grid, allLons& lats, vector<long>& offsets) const
{
    double w = west_, e = east_;

	if(grid->area().empty()){
		if(DEBUG)
			cout << "Reduced::mappedLatitudes Area is empty" << endl;
		return;
	}
	if(grid->area().isOnePoint()){
		if(DEBUG)
			cout << "Reduced::mappedLatitudes Area is one Point" << endl;
            offsets.push_back(0);
            vector<double> lons;
			lons.push_back(w);
			lats.push_back(lons);
		return;
	}
		
	if(globalWestEast_){
		if(same(w,0)){	
    		w = 0;
			e = 360.0;
		}
		else if(same(w,-180.0)){	
    		w = -180.0;
			e = 180.0;
		}
		else {
			throw WrongValue("Reduced::mappedLatitudes - is GLOBAL West-East but West is not matched: ", w);
		}
		if(DEBUG)
			cout << "Reduced::mappedLatitudes is GLOBAL West-East" << endl;
	}
	else {
		if(DEBUG)
			cout << "Reduced::mappedLatitudes NON GLOBAL West-East" << endl;
	}

    long pointsNumber = 0;

    for ( size_t i = northLatitudeIndex_ ; i < northLatitudeIndex_ + gridLatNumber_ ; i++ ) {
        vector<double> lons;
		int pnum = 0;
		if(!globalWestEast_ && grid->gridType() == "gaussian"){
        	pnum = generateLongitudesForSubArea(lons,gridDefinition_[i],west_,east_);
		}
		else{
        	pnum = generateLongitudes(lons,gridDefinition_[i],w,e,globalWestEast_);
		}
        offsets.push_back(pointsNumber);
		lats.push_back(lons);
    	pointsNumber += pnum;
	}
}

int Reduced::generateLongitudesForSubArea(vector<double>& longitudes, int numberOfPoints, double west, double east) const
{
    longitudes.clear();

	bool wrap = false;
    if(west < 0){
		west += 360.0;
		wrap = true;
	}
    double longitude = 0;

    double increment = 360.0 / numberOfPoints;

	if(wrap){
    	for(int i = 0 ; i < numberOfPoints ; i++) {
			if(((longitude > west || same(longitude,west)) && longitude < 360.0) || ( (longitude > 0 || iszero(longitude)) && (longitude < east || same(longitude,east))))
				longitudes.push_back(longitude);
       	 longitude += increment;
		}
	}
	else{
    	for(int i = 0 ; i < numberOfPoints ; i++) {
			if((longitude > west || same(longitude,west)) && (longitude < east || same(longitude,east)))
				longitudes.push_back(longitude);
       	 	longitude += increment;
		}
	}

    return longitudes.size();
}


int Reduced::generateLongitudes(vector<double>& longitudes, int numberOfPoints, double west, double east, bool isGlobalWestEast) const
{
    longitudes.clear();
    double longitude = west, lon = 0;
    ASSERT(west < east);

    double increment = 0;
    if(isGlobalWestEast)
        increment = 360.0 / numberOfPoints;
    else
	// this case is just for reduced latlon
      increment = (east - west) / (numberOfPoints - 1);

	// ssp emos has less precision for increment  irdiwe.F ISTRIDE = JP360 / IILON
//	if(numberOfPoints == 2500)
//		cout << "increment 2500: " << increment << endl;

	if(EMOSLIB_PRECISION){
		increment = floor(increment * 100000.0f)/ 100000.0f;
// this fix is only possible at the moment ecregrid gives 0.14399
		if(numberOfPoints == 2500)
			increment = 0.144;
	}

//		increment = floor(increment * MMULT)/ MMULT;
//		increment = round(increment * 100000)/ 100000;
//		increment = ceil(increment * MULT)/ MULT;
//		increment = rint(increment * 100000)/ 100000;
//	cout.precision(10);
//	cout << "increment " << increment << " numberOfPoints " << numberOfPoints << endl;

    ASSERT(increment > 0);

    for(int i = 0 ; i < numberOfPoints ; i++) {
            if(longitude < 0)
                lon = longitude + 360.0;
            else
                lon = longitude;
			longitudes.push_back(lon);
// ssp accumulate RERR
//            longitude += increment ;
            longitude = increment * (i+1) + west ;
	}
    ASSERT((long)longitudes.size() == numberOfPoints);
    return numberOfPoints;
}

void Reduced::generateGridAsLatLonList(double* lats, double* lons, long* length)  const 
{
	ASSERT((long)calculatedNumberOfPoints_ >= *length);
	long count = 0;

	for (size_t j = 0 ; j < gridLatNumber_; ++j) {
		double lat = latitudes_[j]; 
		vector<double>::const_iterator start = allLons_[j].begin(), it = start, stop = allLons_[j].end();
		for( ; it != stop; it++) {
			lats[count] = lat;
			lons[count] = *it;
		}
	}
	*length = calculatedNumberOfPoints_;
}

void Reduced::generateGrid(vector<Point>& llgrid)  const 
{
	llgrid.clear();
//	llgrid.reserve(calculatedNumberOfPoints_);
	for (size_t j = 0 ; j < gridLatNumber_; ++j) {
		double lat = latitudes_[j]; 
		vector<double>::const_iterator start = allLons_[j].begin(), it = start, stop = allLons_[j].end();
		for( ; it != stop; it++) {
			Point point(lat,*it,it-start,j,getIndex(it-start,j));
			llgrid.push_back(point);
		}
	}
}

void Reduced::generateUnrotatedGrid(vector<Point>& llgrid, const Rotation& rot)  const 
{
	llgrid.clear();
//	llgrid.reserve(calculatedNumberOfPoints_);

	for (size_t j = 0 ; j < gridLatNumber_; ++j) {
		double lat = latitudes_[j];
		vector<double>::const_iterator start = allLons_[j].begin(), it = start, stop = allLons_[j].end();
		for( ; it != stop; it++) {
			Point point(lat,*it,it-start,j,getIndex(it-start,j));
			Point p (rot.unRotate(point));
			llgrid.push_back(p);
		}
	}
	unRotatedArea(llgrid);
}

void Reduced::generateRotatedGrid(vector<Point>& llgrid, const Rotation& rot)  const 
{
	llgrid.clear();
//	llgrid.reserve(calculatedNumberOfPoints_);

	for (size_t j = 0 ; j < gridLatNumber_; ++j) {
		double lat = latitudes_[j];
		vector<double>::const_iterator start = allLons_[j].begin(), it = start, stop = allLons_[j].end();
		for( ; it != stop; it++) {
			Point point(lat,*it,it-start,j,getIndex(it-start,j));
			Point p (rot.rotate(point));
			llgrid.push_back(p);
		}
	}
	unRotatedArea(llgrid);
}

long Reduced::calculateNpts() const
{
	long num = 0;

	if(DEBUG)
		cout << "Reduced::calculateNpts globalWestEast: " << globalWestEast_ << " globalNorthSouth: " << globalNorthSouth_ << endl;

	if(globalWestEast_ && globalNorthSouth_){
		for(size_t i = 0; i < gridLatNumber_; i++)
			num += gridDefinition_[i];
		return num;
	}
	if(gridLatNumber_ == 1 && same(west_,east_))
		return 1;

	if(DEBUG){
		cout << "Reduced::calculateNpts() gridLatNumber_ " << gridLatNumber_ << endl;
		cout << "Reduced::calculateNpts() northLatitudeIndex_ " << northLatitudeIndex_ << endl;
	}

    for(size_t i = 0; i < gridLatNumber_; i++){
        vector<double>::const_iterator start = allLons_[i].begin(), it = start, stop = allLons_[i].end();
        for( ; it != stop; it++)
            num++;
    }

    return num;
}

struct ReducedGridContext : public GridContext {

	int last_j_;

	int north_last_i_;
	int nnorth_last_i_;
	int south_last_i_;
	int ssouth_last_i_;
	int nnnorth_last_i_;
	int nnnnorth_last_i_;
	int sssouth_last_i_;
	int ssssouth_last_i_;
	double last_lat_;
	int n_;
	int s_;
	double north_;
	double south_;
	double northNorth_;
	double southNorth_;
	double northSouth_;
	double southSouth_;

	ReducedGridContext() :
		last_j_(0),
        north_last_i_(0),
        nnorth_last_i_(0),
        south_last_i_(0),
        ssouth_last_i_(0),
        nnnorth_last_i_(0),
        nnnnorth_last_i_(0),
        sssouth_last_i_(0),
        ssssouth_last_i_(0),
        last_lat_(DBL_MAX),
        n_(0),
        s_(0),
        north_(0.0),
        south_(0.0),
	    northNorth_(0.0),
    	southNorth_(0.0),
    	northSouth_(0.0),
    	southSouth_(0.0)
    {}
};

GridContext* Reduced::getGridContext() const
{
	return new ReducedGridContext();
}

double Reduced::conserving(GridContext* ctx, const Point& where, const vector<double>& data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const
{
	ReducedGridContext* c = static_cast<ReducedGridContext*>(ctx);
	ASSERT(c);

	double wlat = where.latitude();
	double wlon = where.longitude();

	long latSize = indexOfLastLat_ + 1;

    long startJ = 0, endJ = 0;
	double northOfCell = 0, southOfCell = 0;
	double northNorth  = 0,  southNorth = 0;
	double northSouth  = 0,  southSouth = 0;

	if(!same(c->last_lat_,wlat)){
		northOfCell = wlat + outNSincrement / 2 ;
		if(northOfCell > latitudes_[0])
//			northOfCell = latitudes_[0];
			return 0;
		southOfCell = wlat - outNSincrement / 2 ;
		if(southOfCell < latitudes_[indexOfLastLat_])
//			southOfCell = latitudes_[indexOfLastLat_];
			return 0;
		c->last_lat_ = wlat;
		int last_j = c->last_j_;
		int nn = -1, sn = -1;
		int ns = -1, ss = -1;
		// up boundary of cell
		long lastLat = findNorthSouthNeighbours(northOfCell,last_j,latitudes_,northNorth,southNorth,nn,sn);
		c->last_j_ = last_j;
		ASSERT(lastLat == indexOfLastLat_);

		// down boundary of cell
		lastLat = findNorthSouthNeighbours(southOfCell,last_j,latitudes_,northSouth,southSouth,ns,ss);
//		 cout << " wlat: " << wlat << " northOfCell: " << northOfCell << " southOfCell: " << southOfCell << endl;
//		 cout << " wlat: " << wlat << " northSouth: " << northSouth << " southSouth: " << southSouth << endl;
//		 cout << " nn: " << nn << " ss: " << ss << endl;

   		 startJ = nn;
   		 if (startJ < 0)
   		   //  startJ = 0;
			return 0;

   		 endJ = ss + 1;
   		 if ( endJ > latSize )
   	     //	endJ = latSize;
			return 0;
		c->last_lat_ = wlat;
        c->n_ = startJ;
        c->s_ = endJ;
		c->northNorth_ = northNorth;
		c->southNorth_ = southNorth;
		c->northSouth_ = northSouth;
		c->southSouth_ = southSouth;
	}
	else{
		startJ = c->n_;
		endJ   = c->s_;
		northNorth = c->northNorth_;
		southNorth = c->southNorth_;
		northSouth = c->northSouth_;
		southSouth = c->southSouth_;
	}

	double westOfCell = wlon - outWEincrement / 2;
	if(westOfCell < 0)
        westOfCell += 360.0;
	double eastOfCell = wlon + outWEincrement / 2;
	if(eastOfCell >= 360.0)
        eastOfCell -= 360.0;

	double westWest = 0, eastWest = 0; 
	double westEast = 0, eastEast = 0;

	vector<double> valuesAlongMeridian;
	valuesAlongMeridian.reserve(endJ - startJ);

	double accumulate = 0;
/* ssp to solve the problem  when west boundary less than 360 and east boundary > 0
   in that case left value is much higher than right 
*/
//		 cout << " startJ: " << startJ << " endJ: " << endJ << endl;

    for( long j = startJ ; j < endJ ; j++)
    {
		int ww = -1, ew = -1;
		int we = -1, ee = -1;
		// left boundary of cell
		long indexOfLastLon = findWestEastNeighbours(westOfCell,c->north_last_i_,allLons_[j],westWest,eastWest,ww,ew);
		/*
		if(bicubic)
			int eew  = ifIndexFirstLon(ew + 1, indexOfLastLon);
			int www  = ifIndexLastLon(ww - 1, indexOfLastLon);
				
		*/
		long kww = get1dIndex(ww,j,scMode);
		long kew = get1dIndex(ew,j,scMode);
		/*
		cout << "ww: " << ww << " ew " << ew << " scMode: " << scMode  << endl;
		cout << "kww: " << kww << " kew " << kew << endl;
		*/
		double leftValue = linear(westOfCell,westWest,data[kww],eastWest,data[kew],missingValue);
		if(leftValue > 7.0){
			cout << "west: " << westOfCell << " j " << j << " value: " << leftValue  << " data w: " << data[kww] << " data e: " << data[kew] << endl;
			cout << "west: " << westOfCell << " j " << j << " westwest: " << westWest  << " eastwest " << eastWest << endl;
		}

		// right boundary of cell
		indexOfLastLon = findWestEastNeighbours(eastOfCell,c->south_last_i_,allLons_[j],westEast,eastEast,we,ee);
		long kwe = get1dIndex(we,j,scMode);
		long kee = get1dIndex(ee,j,scMode);
		double rightValue = linear(eastOfCell,westEast,data[kwe],eastEast,data[kee],missingValue);
		if(rightValue > 7.0){
			cout << " eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" << endl;
			cout << "east: " << eastOfCell << " j " << j << " value: " << rightValue  << " data w: " << data[kwe] << " data e: " << data[kee] << endl;
			cout << "east: " << eastOfCell << " j " << j << " westEast: " << westEast  << " eastEast " << eastEast << endl;
		}
		double value = rightValue - leftValue;	
		if(leftValue > rightValue)
			value = 0;

			if(value > 10)
				cout << "rightValue - leftValue " << value << endl;

		accumulate += value;
		valuesAlongMeridian.push_back(accumulate);
	}
	int lastIndex = valuesAlongMeridian.size() - 1;

	double upValue = linear(northOfCell,northNorth,valuesAlongMeridian[0],southNorth,valuesAlongMeridian[1],missingValue);
//		cout << "northOfCell: " << northOfCell << " northNorth: " << northNorth << " southNorth: " << southNorth << " upValue " << upValue << endl;

	double downValue = linear(southOfCell,northSouth,valuesAlongMeridian[lastIndex-1],southSouth,valuesAlongMeridian[lastIndex],missingValue);

//		cout << "southOfCell: " << southOfCell << " northSouth: " << northSouth << " southSouth: " << southSouth << " downValue " << downValue << endl;

	if((downValue - upValue) > 10 )
		cout << "VALUE " << downValue - upValue << endl;

	return downValue - upValue;
}

void Reduced::cellNS(ReducedGridContext* ctx, double wlat, double outNSincrement, long& startJ, long& endJ) const
{
	ReducedGridContext* c = static_cast<ReducedGridContext*>(ctx);
	ASSERT(c);

	if(!same(c->last_lat_,wlat)){
		c->last_lat_ = wlat;
		int last_j = c->last_j_;

        bool northPole = false;
        bool southPole = false;
		double northOfCell = wlat + outNSincrement / 2 ;
		if(northOfCell > latitudes_[0]){
			northOfCell = latitudes_[0];
			northPole   = true;
		}
		double southOfCell = wlat - outNSincrement / 2 ;
		if(southOfCell < latitudes_[indexOfLastLat_]){
			southOfCell = latitudes_[indexOfLastLat_];
			southPole   = true;
		}
        if(northPole){
            if(southOfCell > northOfCell)
                southOfCell = northOfCell;
        }
        if(southPole){
            if(southOfCell > northOfCell)
                northOfCell = southOfCell;
        }

		double northNorth = 0, southNorth = 0;
		double northSouth = 0, southSouth = 0;
		int nn = -1, sn = -1;
		int ns = -1, ss = -1;
		// up boundary of cell
		long lastLat = findNorthSouthNeighbours(northOfCell,last_j,latitudes_,northNorth,southNorth,nn,sn);
		ASSERT(lastLat == indexOfLastLat_);

		// down boundary of cell
		lastLat = findNorthSouthNeighbours(southOfCell,last_j,latitudes_,northSouth,southSouth,ns,ss);
		c->last_j_ = last_j;
    	startJ = sn;
    	if (startJ < 0)
       	 startJ = 0;

		long latSize = indexOfLastLat_ + 1;
    	endJ = ss;

    	if ( endJ > latSize || endJ <= 0)
       		 endJ = latSize;
//    	if ( endJ > latSize )
//      		 endJ = latSize;

// ----------------------------------------
		if(startJ == 0){
			if((endJ <= startJ))
				endJ = 1;		
		}
		else if(endJ == latSize){
			if((endJ <= startJ))
				startJ = endJ - 1;		
		}
// ----------------------------------------
			
		c->n_ = startJ;
		c->s_ = endJ;
	}
	else{
		startJ = c->n_;
		endJ   = c->s_;
	}
}

Area Reduced::cell(ReducedGridContext* ctx, bool flux, const Point& point, const vector<double>& outLats ,double outWEincrement, long& startJ, long& endJ) const
{
	ReducedGridContext* c = static_cast<ReducedGridContext*>(ctx);
	ASSERT(c);
	double wlon = point.longitude();
	double wlat = point.latitude();
	int jOutIndex = point.jIndex();
	int outLastIndex = outLats.size() - 1;

	double westOfCell = wlon - (outWEincrement * 0.5);
	if(westOfCell < 0)	
		westOfCell += 360.0;
	double eastOfCell = wlon + (outWEincrement * 0.5);
	if(eastOfCell >= 360.0)
		eastOfCell -= 360.0;

	if(!same(c->last_lat_,wlat)){
		c->last_lat_ = wlat;
		int last_j = c->last_j_;

        bool northPole = false;
        bool southPole = false;

		double northOfCell = wlat;
		if(jOutIndex)
			northOfCell = wlat + ((outLats[jOutIndex - 1] - outLats[jOutIndex]) * 0.5);

		if(northOfCell > latitudes_[0] || jOutIndex == 0){
			northOfCell = latitudes_[0];
			northPole   = true;
		}

		double southOfCell = wlat;
		if(jOutIndex < outLastIndex )
			southOfCell = wlat - ((outLats[jOutIndex] - outLats[jOutIndex + 1]) * 0.5);

		if(southOfCell < latitudes_[indexOfLastLat_] || jOutIndex == outLastIndex){
			southOfCell = latitudes_[indexOfLastLat_];
			southPole   =  true;
		}
		else{
		}

        if(northPole){
            if(southOfCell > northOfCell)
                southOfCell = northOfCell;
        }
        if(southPole){
            if(southOfCell > northOfCell)
                northOfCell = southOfCell;
        }
//		cout << "Reduced::cell northOfCell: " << northOfCell << " southOfCell: " << southOfCell << "  pole " << latitudes_[0] << endl;

		double northNorth = 0, southNorth = 0;
		double northSouth = 0, southSouth = 0;
		int nn = -1, sn = -1;
		int ns = -1, ss = -1;
		// up boundary of cell
		long lastLat = findNorthSouthNeighbours(northOfCell,last_j,latitudes_,northNorth,southNorth,nn,sn);
		ASSERT(lastLat == indexOfLastLat_);

		// down boundary of cell
		lastLat = findNorthSouthNeighbours(southOfCell,last_j,latitudes_,northSouth,southSouth,ns,ss);

		c->north_ = northOfCell;
		c->south_ = southOfCell;

		c->last_j_ = last_j;

// first and last points are out of the cell
		if(flux){
    		startJ = nn;
    		endJ = ss + 1;
		}
		else{
    		startJ = sn;
    		endJ = ns + 1;
		}
    	if (startJ < 0)
       	 startJ = 0;

		long latSize = indexOfLastLat_ + 1;
    	if ( endJ > latSize || endJ <= 0)
       		 endJ = latSize;
// ----------------------------------------
		if(startJ == 0){
			if((endJ <= startJ))
				endJ = 1;		
		}
		else if(endJ == latSize){
			if((endJ <= startJ))
				startJ = endJ - 1;		
		}
// ----------------------------------------
		c->n_ = startJ;
		c->s_ = endJ;
	}
	else{
		startJ = c->n_;
		endJ   = c->s_;
	}

	return Area(c->north_,westOfCell,c->south_,eastOfCell);
}

void Reduced::cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const 
{
	areas.clear();
	areasSize.clear();

	size_t latSize = latitudes_.size();
	double middleOfLayer = (latitudes_[0] - latitudes_[1]) / 2;
    for(size_t j = 0; j < latSize; j++) {
		long numPts = gridDefinition_[j];
        if(!numPts)
            continue;
        double westEastInc =  westEastIncrement(numPts);

		double northOfCell = latitudes_[j] + middleOfLayer / 2 ;
		if(northOfCell > latitudes_[0])
			northOfCell = latitudes_[0];
		if( j+1 < latSize)
			middleOfLayer = (latitudes_[j] - latitudes_[j+1]) / 2;
		double southOfCell = latitudes_[j] - middleOfLayer / 2 ;
		if(southOfCell < latitudes_[indexOfLastLat_])
			southOfCell = latitudes_[indexOfLastLat_];

    	for(int i = 0; i < numPts; i++) {
			double westOfCell = allLons_[j][i] - (westEastInc / 2);
			if(westOfCell < 0)	
				westOfCell += 360.0;
			double eastOfCell = allLons_[j][i] + (westEastInc / 2);
			if(eastOfCell >= 360.0)
				eastOfCell -= 360.0;
            Area area(northOfCell,westOfCell,southOfCell,eastOfCell);
//			cout << point << endl;
            areas.push_back(area);
            areasSize.push_back(area.size());
        }
    }
}

double Reduced::fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const
{
	long  startJ = 0, endJ  = 0;
	//double wlat = where.latitude();
	ReducedGridContext* c = static_cast<ReducedGridContext*>(ctx);
	ASSERT(c);
	vector<double> temp;

	Area outCellArea = cell(c,true,where,temp,outWEincrement,startJ,endJ);


	long startI = 0, endI = 0;

	double westOfCell = outCellArea.west();
	double eastOfCell = outCellArea.east();

	double outValue = 0;

    for( long j = startJ ; j < endJ ; j++)
    {
		double westEast = 0, eastEast = 0;
		double westWest = 0, eastWest = 0; 
		int ww = -1, ew = -1;
		int we = -1, ee = -1;

		// left boundary of cell
		long indexOfLastLon = findWestEastNeighbours(westOfCell,c->north_last_i_,allLons_[j],westWest,eastWest,ww,ew);
		// right boundary of cell
		indexOfLastLon = findWestEastNeighbours(eastOfCell,c->south_last_i_,allLons_[j],westEast,eastEast,we,ee);

		int lonSize = indexOfLastLon + 1;

// west ans east out of the cell
   		startI = ww;
   		endI   = ee + 1;

   		int startII = 0;
   		int endII = 0;
   		bool wrap = false;

		if ( endI > lonSize){
			wrap    = true;
       	 	startII = 0;
       	 	endII   = endI - lonSize;
       	 	endI    = lonSize;
    	}

   		if (startI >  endI){
			wrap   = true;
       		endII   = endI;
       		startII = 0;
       		endI   = lonSize;
   		}

		ASSERT(startI >= 0 && endI <= lonSize);
		double areaCheck = 0;
        for( int i = startI ; i < endI ; i++)
        {
			long k = get1dIndex(i,j,scMode);
			double value = data[k];
            if (!same(value,missingValue)){
				if(outCellArea.contain(areas[k])){
					outValue += inputCellSize[k] * value / outCellArea.size();
					areaCheck += inputCellSize[k] / outCellArea.size();
				}
				/*
				else if(outCellArea.intersect(areas[k])){
//					Area common = outCellArea.intersection(areas[k]);
//						double size = common.size();
					double size = 1;
					outValue += size * value / outCellArea.size();
					areaCheck += size / outCellArea.size();
				}
				*/
            }
        }
        if(wrap){
            for( long i = startII ; i < endII; i++)
            {
				long k = get1dIndex(i,j,scMode);
				double value = data[k];
                if (!same(value,missingValue)){
					if(outCellArea.contain(areas[k])){
						outValue += inputCellSize[k] * value / outCellArea.size();
						areaCheck += inputCellSize[k] / outCellArea.size();
					}
					/*
					else if(outCellArea.intersect(areas[k])){
//						Area common = outCellArea.intersection(areas[k]);
//						double size = common.size();
						double size = 1;
						outValue += size * value / outCellArea.size();
						areaCheck += size / outCellArea.size();
					}
					*/
                }
            }
		}
	}

	return outValue;
}

double Reduced::averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double outWEincrement)  const
{
	long  startJ = 0, endJ  = 0;
	ReducedGridContext* c = static_cast<ReducedGridContext*>(ctx);
	ASSERT(c);

//	double wlat = where.latitude();
//	cellNS(c,wlat,outNSincrement,startJ,endJ);
	bool flux = true;
	Area area = cell(c,flux,where,outLats,outWEincrement,startJ,endJ);


	/*
	double wlon = where.longitude();
	double westOfCell = wlon - (outWEincrement / 2);
	if(westOfCell < 0)	
		westOfCell += 360.0;
	double eastOfCell = wlon + (outWEincrement / 2);
	if(eastOfCell >= 360.0)
		eastOfCell -= 360.0;
	*/
	double westOfCell = area.west();
	double eastOfCell = area.east();
	long startI = 0, endI = 0;

	double sum = 0, summ = 0;
	long count = 0;

    for( long j = startJ ; j < endJ ; j++)
    {
		if(!allLons_[j].size()){
//			cout << "Reduced::averageWeighted empty latitude j:  " << j << " " << allLons_[j].size() << endl;
			continue;
		}

		double westEast = 0, eastEast = 0;
		double westWest = 0, eastWest = 0; 
		int ww = -1, ew = -1;
		int we = -1, ee = -1;

		// left boundary of cell
		long indexOfLastLon = findWestEastNeighbours(westOfCell,c->north_last_i_,allLons_[j],westWest,eastWest,ww,ew);
		// right boundary of cell
		indexOfLastLon = findWestEastNeighbours(eastOfCell,c->south_last_i_,allLons_[j],westEast,eastEast,we,ee);

		int lonSize = indexOfLastLon + 1;
		if(flux){
   			startI = ww;
   			endI   = ee + 1;
		}
		else{
   			startI = ew;
   			endI   = we + 1;
		}

   		int startII = 0;
   		int endII = 0;
   		bool wrap = false;

		if ( endI > lonSize){
			wrap    = true;
       	 	startII = 0;
       	 	endII   = endI - lonSize;
       	 	endI    = lonSize;
    	}

   		if (startI >  endI){
			wrap   = true;
       		endII   = endI;
       		startII = 0;
       		endI   = lonSize;
   		}

		ASSERT(startI >= 0 && endI <= lonSize);
        for( int i = startI ; i < endI ; i++)
        {
			long k = get1dIndex(i,j,scMode);
			double value = data[k];
            if (!same(value,missingValue)){
				double weight = weights[k];
				sum += value * weight;
				summ += weight;
				count++;
            }
        }
        if(wrap){
            for( long i = startII ; i < endII; i++)
            {
				long k = get1dIndex(i,j,scMode);
				double value = data[k];
                if (!same(value,missingValue)){
					double weight = weights[k];
					sum += value * weight;
					summ += weight;
					count++;
                }
            }
		}
		/*
		if(!count){
			cout << "Reduced::averageWeighted NPTS: " << allLons_[j].size()  << endl;
			cout << "Reduced::averageWeighted westOfCell: " << westOfCell << " eastOfCell: " << eastOfCell << endl;
			cout << "Reduced::averageWeighted j: " << j << " startI: " << startI << " endI: " << endI << endl;
			cout << "Reduced::averageWeightedj: " << j << " startII: " << startII << " endII: " << endII << endl;
		}
		*/
	}

	if(!count){
//			cout << "Reduced::averageWeighted startJ: " << startJ << " endJ: " << endJ << endl;
//			cout << "----------------------------------------------------" << endl;
		return missingValue;
	}

	if(iszero(sum))
		return 0;

	return sum / summ;

}

void Reduced::findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex )  const
{
	ReducedGridContext* c = static_cast<ReducedGridContext*>(ctx);
	ASSERT(c);
	int ww = 0, ee = 0;
	double westWest = 0., eastEast = 0., eastWest = 0., westEast = 0.;
cout << j << " size of lons " << allLons_[j].size() << endl; //////////////////////////////////////
	// left boundary
	long indexOfLastLon = findWestEastNeighbours(west,c->north_last_i_,allLons_[j],westWest,eastWest,ww,westLongitudeIndex);
	// right boundary
	indexOfLastLon = findWestEastNeighbours(east,c->south_last_i_,allLons_[j],westEast,eastEast,eastLongitudeIndex,ee);
}

double Reduced::averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double outWEincrement)  const
{
	long  startJ = 0, endJ  = 0;
	ReducedGridContext* c = static_cast<ReducedGridContext*>(ctx);
	ASSERT(c);

//	double wlat = where.latitude();
//	cellNS(c,wlat,outNSincrement,startJ,endJ);

	bool flux = true;
	Area area = cell(c,flux,where,outLats,outWEincrement,startJ,endJ);


	/*
	double wlon = where.longitude();
	double westOfCell = wlon - (outWEincrement / 2);
	if(westOfCell < 0)	
		westOfCell += 360.0;
	double eastOfCell = wlon + (outWEincrement / 2);
	if(eastOfCell >= 360.0)
		eastOfCell -= 360.0;
	*/
	double westOfCell = area.west();
	double eastOfCell = area.east();
	long startI = 0, endI = 0;

	double sum = 0, summ = 0;
	long count = 0;

    for( long j = startJ ; j < endJ ; j++)
    {
		if(!allLons_[j].size()){
			continue;
		}

		double westEast = 0, eastEast = 0;
		double westWest = 0, eastWest = 0; 
		int ww = -1, ew = -1;
		int we = -1, ee = -1;

		// left boundary of cell
		long indexOfLastLon = findWestEastNeighbours(westOfCell,c->north_last_i_,allLons_[j],westWest,eastWest,ww,ew);
		// right boundary of cell
		indexOfLastLon = findWestEastNeighbours(eastOfCell,c->south_last_i_,allLons_[j],westEast,eastEast,we,ee);

		int lonSize = indexOfLastLon + 1;
		if(flux){
   			startI = ww;
   			endI   = ee + 1;
		}
		else{
   			startI = ew;
   			endI   = ee;
		}

   		int startII = 0;
   		int endII = 0;
   		bool wrap = false;

		if ( endI > lonSize){
			wrap    = true;
       	 	startII = 0;
       	 	endII   = endI - lonSize;
       	 	endI    = lonSize;
    	}

   		if (startI >  endI){
			wrap   = true;
       		endII   = endI;
       		startII = 0;
       		endI   = lonSize;
   		}

		ASSERT(startI >= 0 && endI <= lonSize);
        for( int i = startI ; i < endI ; i++)
        {
			long k = get1dIndex(i,j,scMode);
			double value = data[k];
            if (!same(value,missingValue)){
				double weight = weights[k];
				if(dataLsmOut[where.k1dIndex()] != dataLsmIn[k])
					weight *= LSM_FACTOR;
				sum += value * weight;
				summ += weight;
				count++;
            }
        }
        if(wrap){
            for( long i = startII ; i < endII; i++)
            {
				long k = get1dIndex(i,j,scMode);
				double value = data[k];
                if (!same(value,missingValue)){
					double weight = weights[k];
					if(dataLsmOut[where.k1dIndex()] != dataLsmIn[k])
						weight *= LSM_FACTOR;
					sum += value * weight;
					summ += weight;
					count++;
                }
            }
		}
	}

	if(!count){
		return missingValue;
	}

	if(iszero(sum))
		return 0;

	return sum / summ;
}

void Reduced::nearest4(GridContext* ctx,const Point& where, vector<Point>& result) const
{
	double west = 0, east = 0, north = 0, south = 0;
	int n = -1, s = -1;
	long k = -1;

	ReducedGridContext* c = static_cast<ReducedGridContext*>(ctx);
	ASSERT(c);

    int north_last_i = c->north_last_i_;
    int south_last_i = c->south_last_i_;

	result.clear();

	double wlat = where.latitude();
	double wlon = where.longitude();

    if(!same(c->last_lat_, wlat)){
        c->last_lat_ = wlat;
		long lastLat = findNorthSouthNeighbours(wlat,c->last_j_,latitudes_,north,south,n,s);
		ASSERT(lastLat == indexOfLastLat_);
		c->n_ = n;
		c->s_ = s;
		c->north_ = north;
		c->south_ = south;
    }
	else{
		n = c->n_;
		s = c->s_;
		north = c->north_;
		south = c->south_;
	}

    long indexOfLastLonN = -1, indexOfLastLonS = -1;;
	int wiN = -1, eiN = -1;

	if(n != -1 && allLons_[n].size() ) {
		// N
		// Find neighbours on N latitude
		indexOfLastLonN = findWestEastNeighbours(wlon,north_last_i,allLons_[n],west,east,wiN,eiN);
		c->north_last_i_ = north_last_i;
		// 0
		k = getIndex(wiN,n);
		result.push_back( Point(north,west,wiN,n,k) );
		// 1
		k = getIndex(eiN,n);
		result.push_back( Point(north,east,eiN,n,k));
	}
	int wiS = -1, eiS = -1;

	if(s != -1 && allLons_[s].size()) {
		// S
		// Find neighbours on S latitude
		indexOfLastLonS = findWestEastNeighbours(wlon,south_last_i,allLons_[s],west,east,wiS,eiS);
        c->south_last_i_ = south_last_i;
		// 2
		k = getIndex(wiS,s);
		result.push_back( Point(south,west,wiS,s,k));
		// 3
		k = getIndex(eiS,s);
		result.push_back( Point(south,east,eiS,s,k));
	}
}


void Reduced::nearestPts(GridContext* ctx,const Point& where, vector<FieldPoint>& result, const vector<double>& data, int scMode, int howMany) const
{
	double west  = 0., east = 0., north = 0., south = 0.;
	double value = 0.;
	int n = -1, s = -1;
	long k = -1;

	ReducedGridContext* c = static_cast<ReducedGridContext*>(ctx);
	ASSERT(c);

    int north_last_i = c->north_last_i_;
    int south_last_i = c->south_last_i_;

	result.clear();

	double wlat = where.latitude();
	double wlon = where.longitude();

    if(!same(c->last_lat_, wlat)){
        c->last_lat_ = wlat;
		long lastLat = findNorthSouthNeighbours(wlat,c->last_j_,latitudes_,north,south,n,s);
		ASSERT(lastLat == indexOfLastLat_);
		c->n_ = n;
		c->s_ = s;
		c->north_ = north;
		c->south_ = south;
    }
	else{
		n = c->n_;
		s = c->s_;
		north = c->north_;
		south = c->south_;
	}

    long indexOfLastLonN = -1, indexOfLastLonS = -1;;
	int wiN = -1, eiN = -1;

//	cout << " north " << allLons_[n].size() << endl;

// Because of Reduced Lat-Lon . It is possible to have 0 points on lat
	if(n != -1 && allLons_[n].size() ) {
		// N
		// Find neighbours on N latitude
		indexOfLastLonN = findWestEastNeighbours(wlon,north_last_i,allLons_[n],west,east,wiN,eiN);
		c->north_last_i_ = north_last_i;
		// 0
		k = get1dIndex(wiN,n,scMode);
		result.push_back( FieldPoint(north,west,wiN,n,k,data[k]) );
		// 1
		k = get1dIndex(eiN,n,scMode);
		result.push_back( FieldPoint(north,east,eiN,n,k,data[k]));
	}

	int wiS = -1, eiS = -1;

//	cout << " south " << allLons_[s].size() << endl;

// Because of Reduced Lat-Lon . It is possible to have 0 points on lat
	if(s != -1 && allLons_[s].size()) {
		// S
		// Find neighbours on S latitude
		indexOfLastLonS = findWestEastNeighbours(wlon,south_last_i,allLons_[s],west,east,wiS,eiS);
        c->south_last_i_ = south_last_i;
		// 2
		k = get1dIndex(wiS,s,scMode);
		result.push_back( FieldPoint(south,west,wiS,s,k,data[k]));
		// 3
		k = get1dIndex(eiS,s,scMode);
		result.push_back( FieldPoint(south,east,eiS,s,k,data[k]));
	}

	int  lonIndex = -1, latIndex = -1;
	double longitude;
    long indexOfLastLonNN   = -1, indexOfLastLonSS   = -1;
    long indexOfLastLonNNN  = -1, indexOfLastLonSSS  = -1;
//    long indexOfLastLonNNNN = -1, indexOfLastLonSSSS = -1;
	int wiNN   = -1, eiNN   = -1;
	//int wiNNN  = -1, eiNNN  = -1;
	int wiNNNN = -1, eiNNNN = -1;
	int wiSS   = -1, eiSS   = -1;
	int wiSSS  = -1, eiSSS  = -1;
	int wiSSSS = -1, eiSSSS = -1;

	int ilon12=0, ilon13=0, ilon6=0,  ilon7=0,  ilon8=0;
    int ilon9=0,  ilon14=0, ilon15=0, ilon16=0, ilon21=0;
    int ilon35=0, ilon22=0, ilon34=0, ilon23=0, ilon33=0;
    int ilon24=0, ilon32=0, ilon25=0, ilon31=0, ilon26=0;

	if (howMany > 4) 
    {

		if(result.size() < 4)
			return;
	/*
		Numbering of the points (I is the interpolation point):

        NN           12      4       5      13

        N            6       0       1      7
                                (I)
        S            8       2       3      9

        SS           14      10      11     15

	*/
	 	/* if NN  or SS row missing - return */
		if(n - 1 < 0 || s + 1 > indexOfLastLat_)
			return;
    	int nnorth_last_i = c->nnorth_last_i_;
    	int ssouth_last_i = c->ssouth_last_i_;


 		/* Find neighbours on NN latitude */
		indexOfLastLonNN = findWestEastNeighbours(wlon,nnorth_last_i,allLons_[n-1],west,east,wiNN,eiNN);
        c->nnorth_last_i_ =  nnorth_last_i;

		// 4
		latIndex = n - 1;
		k = get1dIndex(wiNN,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],west,wiNN,latIndex,k,data[k]));

		// 5
//		latIndex = n - 1;
		k = get1dIndex(eiNN,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],east,eiNN,latIndex,k,data[k]));

		// 6
		latIndex  = n;
		lonIndex  = ifIndexLastLon(wiN - 1, indexOfLastLonN);
		ilon6     = lonIndex;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));

		// 7
//		latIndex  = n;
		lonIndex  = ifIndexFirstLon(eiN + 1, indexOfLastLonN);
		ilon7     = lonIndex;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));

		// 8
		lonIndex  = ifIndexLastLon(wiS - 1, indexOfLastLonS);
		ilon8     = lonIndex;
		latIndex  = s;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));


		// 9
		lonIndex  = ifIndexFirstLon(eiS + 1, indexOfLastLonS);
		ilon9     = lonIndex;
		latIndex  = s;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));

 		/* Find neighbours on SS latitude */
		indexOfLastLonSS  = findWestEastNeighbours(wlon,ssouth_last_i,allLons_[s+1],west,east,wiSS,eiSS);
        c->ssouth_last_i_ = ssouth_last_i;

		// 10
		latIndex = s + 1;
		k = get1dIndex(wiSS,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],west,wiSS,latIndex,k,data[k]));
		// 11
		latIndex = s + 1;
		k = get1dIndex(eiSS,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],east,eiSS,latIndex,k,data[k]));
	}

	if (howMany > 12) {
		// 12
		lonIndex  = ifIndexLastLon(wiNN - 1, indexOfLastLonNN);
		ilon12    = lonIndex;
		latIndex  = n - 1;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));

		// 13
		lonIndex  = ifIndexFirstLon(eiNN + 1, indexOfLastLonNN);
		ilon13    = lonIndex;
		latIndex  = n - 1;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));

		// 14
		lonIndex  = ifIndexLastLon(wiSS - 1, indexOfLastLonSS);
		ilon14    = lonIndex;
		latIndex  = s + 1;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));

		// 15
		lonIndex  = ifIndexFirstLon(eiSS + 1, indexOfLastLonSS);
		ilon15    = lonIndex;
		latIndex  = s + 1;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
	}

   /*
       Numbering of the points (I is the interpolation point):
       NNNN    36  37   38   39      40   41   42  43

       NNN     63  16   17   18      19   20   21  44

       NN      62  35   12    4       5   13   22  45

       N       61  34    6    0       1    7   23  46
                                 (I)
       S       60  33    8    2       3    9   24  47

       SS      59  32   14   10      11   15   25  48

       SSS     58  31   30   29      28   27   26  49

       SSSS    57  56   55   54      53   52   51  50
    */
	
	int winnn = -1, einnn = -1;

	if (howMany > 16) {
	 	/* if NNN or SSS row missing - return */
		if(n - 2 < 0 || s + 2 > indexOfLastLat_)
			return;

 		/* Find neighbours on NNN latitude */
    	int nnnorth_last_i = c->nnnorth_last_i_;
		indexOfLastLonNNN = findWestEastNeighbours(wlon,nnnorth_last_i,allLons_[n-2],west,east,winnn,einnn);
        c->nnnorth_last_i_ = nnnorth_last_i;
		// 16 - 21
		latIndex  = n - 2;
		int ilon17  = ifIndexLastLon(winnn - 1, indexOfLastLonNNN);
		// 16
		lonIndex  = ifIndexLastLon(ilon17 - 1, indexOfLastLonNNN);
		ilon16    = lonIndex;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 17
		lonIndex  = ilon17;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 18
		lonIndex  = winnn;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 19
		lonIndex  = einnn;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 20
		lonIndex  = ifIndexFirstLon(lonIndex + 1, indexOfLastLonNNN);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 21
		lonIndex  = ifIndexFirstLon(lonIndex + 1, indexOfLastLonNNN);
		ilon21    = lonIndex;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 22 - 25
		// 22
		latIndex  = n - 1;
		lonIndex  = ifIndexFirstLon(ilon13 + 1, indexOfLastLonNN);
		ilon22    = lonIndex;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 23
		latIndex  = n;
		lonIndex  = ifIndexFirstLon(ilon7 + 1, indexOfLastLonN);
		ilon23    = lonIndex;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 24
		latIndex  = s;
		lonIndex  = ifIndexFirstLon(ilon9 + 1, indexOfLastLonS);
		ilon24    = lonIndex;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 25
		latIndex  = s + 1;
		lonIndex  = ifIndexFirstLon(ilon15 + 1, indexOfLastLonSS);
		ilon25    = lonIndex;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 26 - 31
 		/* Find neighbours on SSS latitude */
		latIndex       = s + 2;
        int sssouth_last_i = c->sssouth_last_i_;
		indexOfLastLonSSS = findWestEastNeighbours(wlon,sssouth_last_i,allLons_[latIndex],west,east,wiSSS,eiSSS);
        c->sssouth_last_i_ = sssouth_last_i ;
		int ilon27  = ifIndexFirstLon(eiSSS + 1, indexOfLastLonSSS);
		// 26
		lonIndex  = ifIndexFirstLon(ilon27 + 1, indexOfLastLonSSS);
		ilon26    = lonIndex;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 27
		lonIndex  = ilon27;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 28
		lonIndex  = eiSSS;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 29
		lonIndex  = wiSSS;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 30
		lonIndex  = ifIndexLastLon(wiSSS - 1, indexOfLastLonSSS);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 31
		lonIndex  = ifIndexLastLon(lonIndex - 1, indexOfLastLonSSS);
		ilon31    = lonIndex;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 32
		latIndex  = s + 1;
		lonIndex  = ifIndexLastLon(ilon14 - 1, indexOfLastLonSS);
		ilon32    = lonIndex;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 33
		latIndex  = s;
		lonIndex  = ifIndexLastLon(ilon8 - 1, indexOfLastLonSS);
		ilon33    = lonIndex;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 34
		latIndex  = n;
		lonIndex  = ifIndexLastLon(ilon6 - 1, indexOfLastLonSS);
		ilon34    = lonIndex;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 35
		latIndex  = n - 1;
		lonIndex  = ifIndexLastLon(ilon12 - 1, indexOfLastLonSS);
		ilon35    = lonIndex;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
	}
	if (howMany > 36) {
	 	/* if NNNN or SSSS row missing - return */
		if(n - 3 < 0 || s + 3 > indexOfLastLat_)
			return;

    	int nnnnorth_last_i = c->nnnnorth_last_i_;
 		/* Find neighbours on NNNN latitude */
		long indexOfLastLonNNNN = findWestEastNeighbours(wlon,nnnnorth_last_i,allLons_[n-3],west,east,wiNNNN,eiNNNN);
        c->nnnnorth_last_i_     = nnnnorth_last_i;

		// 36 - 43
		int ilon38  = ifIndexLastLon(wiNNNN - 1, indexOfLastLonNNNN);
		int ilon37  = ifIndexLastLon(ilon38 - 1, indexOfLastLonNNNN);
		// 36
		lonIndex  = ifIndexLastLon(ilon37 - 1, indexOfLastLonNNNN);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 37
		lonIndex  = ilon37;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 38
		lonIndex  = ilon38;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 39
		latIndex  = n - 3;
		lonIndex  = wiNNNN;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 40
		lonIndex  = eiNNNN;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 41 - 43
		for( int j = 0; j < 3; j++){
			lonIndex  = ifIndexFirstLon(lonIndex + 1, indexOfLastLonNNNN);
			longitude = allLons_[latIndex][lonIndex];
			k = get1dIndex(lonIndex,latIndex,scMode);
			result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		}
		// 44 - 49
		// 44
		latIndex  = n - 2;
		lonIndex  = ifIndexFirstLon(ilon21 + 1, indexOfLastLonNNN);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 45
		latIndex  = n - 1;
		lonIndex  = ifIndexFirstLon(ilon22 + 1, indexOfLastLonNN);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 46
		latIndex  = n;
		lonIndex  = ifIndexFirstLon(ilon23 + 1, indexOfLastLonN);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 47
		latIndex  = s;
		lonIndex  = ifIndexFirstLon(ilon24 + 1, indexOfLastLonS);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 48
		latIndex  = s + 1;
		lonIndex  = ifIndexFirstLon(ilon25 + 1, indexOfLastLonS);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 49
		latIndex  = s + 2;
		lonIndex  = ifIndexFirstLon(ilon26 + 1, indexOfLastLonS);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 50 - 57
     	int ssssouth_last_i = c->ssssouth_last_i_;
		latIndex  = s + 3;
 		/* Find neighbours on SSSS latitude */
		long indexOfLastLonSSSS = findWestEastNeighbours(wlon,ssssouth_last_i,allLons_[latIndex],west,east,wiSSSS,eiSSSS);
        c->ssssouth_last_i_ = ssssouth_last_i;
		int ilon52  = ifIndexFirstLon(eiSSSS + 1, indexOfLastLonSSSS);
		int ilon51  = ifIndexFirstLon(ilon52 + 1, indexOfLastLonSSSS);
		// 50
		lonIndex  = ifIndexFirstLon(ilon51 + 1, indexOfLastLonSSSS);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 51
		lonIndex  = ilon51;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,value));
		// 52
		lonIndex  = ilon52;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 53
		lonIndex  = eiSSSS;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 54
		lonIndex  = wiSSSS;
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 55 - 57
		for( int j = 0; j < 3; j++){
			lonIndex  = ifIndexLastLon(lonIndex - 1, indexOfLastLonSSSS);
			longitude = allLons_[latIndex][lonIndex];
			k = get1dIndex(lonIndex,latIndex,scMode);
			result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 58 - 63
		}
		// 58
		latIndex  = s + 2;
		lonIndex  = ifIndexLastLon(ilon31 - 1, indexOfLastLonSSS);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 59
		latIndex  = s + 1;
		lonIndex  = ifIndexLastLon(ilon32 - 1, indexOfLastLonSS);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 60
		latIndex  = s;
		lonIndex  = ifIndexLastLon(ilon33 - 1, indexOfLastLonS);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 61
		latIndex  = n;
		lonIndex  = ifIndexLastLon(ilon34 - 1, indexOfLastLonN);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 62
		latIndex  = n - 1;
		lonIndex  = ifIndexLastLon(ilon35 - 1, indexOfLastLonNN);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));
		// 63
		latIndex  = n - 2;
		lonIndex  = ifIndexLastLon(ilon16 - 1, indexOfLastLonNNN);
		longitude = allLons_[latIndex][lonIndex];
		k = get1dIndex(lonIndex,latIndex,scMode);
		result.push_back( FieldPoint(latitudes_[latIndex],longitude,lonIndex,latIndex,k,data[k]));

	}

}

void Reduced::printOffsets(ostream& out) const
{
	out << "Offsets ~" << endl; 
	vector<long>::const_iterator iter = offsets_.begin(), stop = offsets_.end();
		int count = 1;
		int j     = 1;
		for( ; iter != stop; iter++){
			out << count++ << ". [Lat: " << latitudes_[j] << " <-> Pts: " << *iter << "] ";
			if (j == 5) {
				j = 0;
				out << endl;
			}
			j++;
		}
//	copy(offsets_.begin(),offsets_.end(), ostream_iterator < mapLatOffset::value_type >(out," \n"));
}

int Reduced::getLatitudeOffset(double latitude) const
{
// ssp this must be faster
    for(size_t i = 0 ; i < gridLatNumber_ ; i++){
        if(same(latitudes_[i],latitude))
            return offsets_[i];
	}

    throw WrongValue("Regular::getLatitudeOffset Latitude",latitude);
	return 0;
}

long Reduced::getLatitudeOffset(double latitude, long& current) const
{
    for(size_t ii = 0 ; ii < gridLatNumber_ ; ii++){
		long i = (ii + current) % gridLatNumber_;
        if(same(latitudes_[i],latitude)){
			current = i;
            return offsets_[i];
		}
	}

    throw WrongValue("Regular::getLatitudeOffset Latitude",latitude);
	return 0;
}

void Reduced::getOffsets(vector<int>& offsets) const
{
	offsets.clear();
	offsets.assign(offsets_.begin(), offsets_.end() );
}

void Reduced::printMappedLatitudes(ostream& out) const
{
	out << "Mapped Latitudes ~" << endl; 
		int count = 0;
	for (size_t j = 0 ; j < gridLatNumber_; ++j) {
			++count;
			out << endl << count << ".  " <<"Lat: " << latitudes_[j] <<  " Point numbers: " << allLons_[j].size() << endl << "[";
			vector<double>::const_iterator it = allLons_[j].begin(), st = allLons_[j].end();
			for( ; it != st; it++)
				out << *it << "  ";
//				out << latitudes_[j] << "," << *it << " " ;
			out << "] " << endl;
		}
}

long Reduced::findNorthSouthNeighboursFast(double wlat, const vector<double>& latitudes, double& north, double& south, int& n, int& s) const
{
	long lastLat = latitudes.size() - 1;
	ASSERT(lastLat);

	long guess = int((90.0 - wlat) / delta_); 

//	long lastLat = indexOfLastLat_;
    for ( int j = guess ; j < lastLat ;  j++ )
    {
        if((wlat < latitudes[j] || same(wlat,latitudes[j])) && wlat > latitudes[j+1]) {
            north = latitudes[j];
            south = latitudes[j+1];
            n = j;
            s = j + 1;
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
    
    throw OutOfRange(lastLat,n );
    return 0;
}

long Reduced::findWestEastNeighboursFast(double wlon, int pointsNumber, const vector<double>& longitudes, double& west, double& east, int& w, int& e) const
{
	// added because rotated grid can have 360.0
	if(same(wlon,360.0))
		wlon = 0.;
 	wlon += ROUNDING_FACTOR;

	double step = 360.0 / pointsNumber;

//	w = ifIndexFirstLon(int(wlon / step), pointsNumber - 1);

	w = int(wlon / step);
	e = ifIndexLastLon(w + 1, pointsNumber - 1);

	west = longitudes[w];
	east = longitudes[e];

	return pointsNumber - 1;
}

void Reduced::print(ostream& out) const
{
	if(getenv("ECREGRID_PRINT_LATLON_REDUCED")){
		out << "Reduced: " << endl; 
		printMappedLatitudes(out);
		printOffsets(out);
	}
}
