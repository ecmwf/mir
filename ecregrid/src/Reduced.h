/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Reduced_H
#define Reduced_H

#ifndef GridType_H
#include "GridType.h"
#endif

#ifndef Exception_H
#include "Exception.h"
#endif


typedef vector< vector<double> > allLons;

// Forward declarations
class Grid;
class Rotation;
class ReducedGridContext;
class Area;


class Reduced : public GridType {
public:

// -- Exceptions
	// None

// -- Contructors

	explicit Reduced(const Grid* grid);
	explicit Reduced(const Grid* grid, const long* gridDefinition, size_t gridLatNumber);

// -- Destructor

	virtual ~Reduced(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	long operator()(int i,int j) const;

// -- Methods
	void nearest4(GridContext* ctx,const Point& where, vector<Point>& result) const;

// -- Overridden methods
	virtual bool rounding() const { return true; }

	void    getOffsets(vector<int>& offsets) const;
	int     getLatitudeOffset(double lat) const;
	long    getLatitudeOffset(double lat, long& current) const;
	double* accumulatedDataWestEast(const double* data, unsigned long dataLength) const;
	void accumulatedDataWestEast(const double* data, unsigned long dataLength, vector<double>& newData) const;
	virtual void reOrderData(const vector<double>& data_in, vector<double>& data_out, int scMode) const;

	void    nearestPts(GridContext*,const Point& where, vector<FieldPoint>& nearests, const vector<double>& data, int scMode, int howMany) const;

	void nearestIndexed(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const;

	double conserving(GridContext* ctx, const Point& where, const vector<double>& data, int scMode, double missingValue, double incNs, double incWe)  const;

//------------------------------------------
	void    generateGrid(vector<Point>& llgrid)  const;
	void    generateUnrotatedGrid(vector<Point>& llgrid, const Rotation& rot)  const;
	void    generateRotatedGrid(vector<Point>& llgrid, const Rotation& rot)  const;
	void    generateGridAsLatLonList(double* lats, double* lons, long* length)  const; 
//------------------------------------------

	long    calculateNpts()  const ;
	unsigned long calculatedNumberOfPoints()  const { return calculatedNumberOfPoints_; }
 
// -- Methods
	void cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const; 
	double fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const;

	double averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double incWe)  const;
	double averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double outWEincrement)  const;

	size_t      gridLatNumber()  const { return gridLatNumber_ ; } 

	size_t  getGridDefinition(vector<long>& v) const;


	void   mappedLatitudes(const Grid* grid, allLons& mappedLats, vector<long>& offsets)  const ;

	int lengthOfFirstLat() const  { return gridDefinition_[0]; } ;
	int lengthOfLastLat() const   { return gridDefinition_[gridLatNumber_ -1]; } ;
	const vector<long>& gridDefinition() const { return gridDefinition_; }

	long getIndex(int i, int j) const;

	long get1dIndex(int i, int j, int scanMode) const {
		ASSERT(i >= 0);
		ASSERT(j >= 0);

		int jj;
        if(scanMode == 1)
            jj = j;
        else if(scanMode == 2)
            jj = gridLatNumber_ - 1 - j;
        else
            throw NotImplementedFeature("Regular::get1dIndex - scanning mode not available ");				

//		cout << "Reduced offset_ " << offset_ << " i " << i << " value " << data[offset_ + i ] << endl;
		if(offsets_[jj] + i >= (long)calculatedNumberOfPoints_){
           throw OutOfRange(offsets_[jj] + i,calculatedNumberOfPoints_);
        }
		return offsets_[jj] + i ;
	}

	void addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const;

	int generateLongitudes(vector<double>& longitudes, int numberOfPoints, double west, double east, bool globalWE) const;

	int generateLongitudesForSubArea(vector<double>& longitudes, int numberOfPoints, double west, double east) const;

	void findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex )  const;

protected:

// -- Methods
	
	void print(ostream&) const;
	void printMappedLatitudes(ostream&) const;
	void printOffsets(ostream&) const;

// -- Overridden methods

	virtual GridContext* getGridContext() const;	

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	Reduced(const Reduced&);
	Reduced& operator=(const Reduced&);

// -- Members
	allLons        allLons_;
	vector<long>   offsets_;

	vector<long>   gridDefinition_;
	vector<double> latitudes_;
	size_t         gridLatNumber_;

	long   indexOfLastLat_;
	size_t northLatitudeIndex_;

	unsigned long calculatedNumberOfPoints_;

	double delta_;
	bool   fast_;

	double west_;
	double east_;

// -- Methods
	void cellNS(ReducedGridContext* ctx, double wlat, double outNSincrement, long& startJ, long& endJ) const;

//	Area cell(ReducedGridContext* ctx, const Point& point, double outNSincrement,double outWEincrement, long& startJ, long& endJ) const;
	Area cell(ReducedGridContext* ctx, bool flux, const Point& point, const vector<double>& outLats, double outWEincrement, long& startJ, long& endJ) const;

	double westEastIncrement(long westEastNumPts, const string& type) const{
        if(!globalWestEast_ && type == "latlon")
       		return (east_ - west_) / (westEastNumPts - 1);

        return 360.0 / westEastNumPts;
    }

	double westEastIncrement(long westEastNumPts) const{
// ssp to improve with non global case
//        if(westEastGlobal_)
            return 360.0 / westEastNumPts;
//       return (east_ - west_) / (westEastNumPts - 1);
    }

	long findNorthSouthNeighboursFast(double wlat, const vector<double>& latitudes, double& north, double& south, int& n, int& s) const;
	long findWestEastNeighboursFast(double wlon, int pointsNumber, const vector<double>& longitudes, double& west, double& east, int& w, int& e) const;


// -- Friends

	friend ostream& operator<<(ostream& s,const Reduced& p)
		{ p.print(s); return s; }
};

#endif
