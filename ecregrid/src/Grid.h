/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Grid_H
#define Grid_H

#ifndef   machine_H
#include "machine.h"
#endif

#ifndef Area_H
#include "Area.h"
#endif

#include <vector>

// Forward declarations
class Point;
class FieldPoint;

class RegularLatLon;
class RegularLatLonCellCentered;
class RegularGaussian;
class PseudoRegularGaussian;
class ReducedGaussian;
class ReducedLatLon;
class RotatedRegularLatLon;
class ListOfPoints;
class PolarStereographic;
class GridContext;


class Grid {
public:

// -- Exceptions
	// None

// -- Contructors

	Grid(double north=0.0, double west=0.0, double south=0.0, double east=0.0);
	Grid(const Area& other);

// -- Destructor
	virtual ~Grid();

// -- Convertors
	// None

// -- Operators
	bool operator==(const Grid& other) const;
	bool operator!=(const Grid& other) const;

// -- Methods

	virtual GridContext* getGridContext() const { return 0; };

	virtual void nearest4pts(GridContext* ctx,const Point& where, vector<Point>& result) const = 0;

	virtual void nearestPoints(GridContext*,const Point& where,vector<FieldPoint>& nearests, const vector<double>& data, int scMode, int npts) const = 0;
	virtual void nearestsByIndex(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const = 0;
	virtual double conserving(GridContext* ctx, const Point& where, const vector<double>& data, int scMode, double missingValue, double incNs, double incWe)  const = 0;

	virtual void cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const = 0; 

	virtual double fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const = 0;

	virtual double averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double incWe)  const = 0;
	virtual double averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data,const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double incWe)  const = 0;
//-------------------------------

	double north() const;
	double west()  const;
	double east()  const;
	double south() const;

	Area area() const { return area_; }

	Area    areaFix(const Grid&) const;

	virtual bool equals(const Grid& other)            const = 0;
	virtual bool sameAs(const RegularLatLon& other)   const  { return false; }
	virtual bool sameAs(const RegularLatLonCellCentered& other)   const  { return false; }
	virtual bool sameAs(const ReducedLatLon& other)   const  { return false; }
	virtual bool sameAs(const RotatedRegularLatLon& other)   const  { return false; }
	virtual bool sameAs(const ReducedGaussian& other) const  { return false; }
	virtual bool sameAs(const RegularGaussian& other) const  { return false; }
	virtual bool sameAs(const PseudoRegularGaussian& other) const  { return false; }
	virtual bool sameAs(const PolarStereographic& other) const  { return false; }
	virtual bool sameAs(const ListOfPoints& other)    const  { return false; }

	virtual void latitudes(vector<double>& lats) const = 0;
	virtual void northHemisphereLatitudes(vector<double>& latitudes) const = 0;

// For grib headers
	virtual double   westEastIncrement()                      const = 0;
	virtual double   northSouthIncrement()                    const = 0;
	virtual int      poleEquatorNumberOfPoints()              const = 0; 
	virtual double   latPole()     const { return 0; }
	virtual double   lonPole()     const { return 0; }
	virtual bool     reduced()     const { return false; } 
	virtual bool     rotated()     const { return false; } 
	virtual bool     stretched()   const { return false; } 
	virtual string   gridType()                               const = 0;
	virtual string   composedName()                           const = 0;

//
	virtual string   predefinedLsmFileName()   const = 0;
	virtual string   lsmFileType() const  { return "grib"; }
	 
// ------------
	virtual string   coeffInfo()                              const = 0;

			Grid*    newGrid()                           const;
	virtual Grid*    newListGrid(const Grid& other) const;
	virtual Grid*    newGrid(const Grid& other)          const = 0;
	virtual Grid*    newGrid(const Area& other)          const = 0;
	virtual Grid*    getGlobalGrid()                     const = 0;
	virtual unsigned long  calculatedNumberOfPoints()    const = 0;
    virtual int westEastNumberOfPoints()            const = 0;
    virtual int northSouthNumberOfPoints()   		 const = 0;

	virtual Grid&    getGridType();

	virtual void  generateGrid1D(vector<Point>&) const = 0;
	virtual void  generateGrid1D(vector<Point>& ll, const Grid& grid) const = 0;

// Adjust area ------------------------------------------------
	virtual  Area  adjustArea(const Area& area,bool globalWE, bool globalNS)  const = 0;
	virtual  void  adjustAreaWestEast( double& west, double& east, double increment) const;
	virtual  void  adjustAreaWestEastMars( double& west, double& east, double increment) const;
	virtual  Area  setGlobalArea()               const = 0;
	virtual  Area  setGlobalArea(double west)    const = 0;
	virtual  Area  fitToGlobalArea(double west)  const = 0;
	bool           isGlobal()                    const;
	virtual void setGlobalWestEast( double& west, double& east, double increment) const;
	virtual  void  setGlobalNorthSouth(double& north, double& south) const = 0;
	virtual  bool  isGlobalWestEast()            const = 0;
	virtual  bool  isGlobalNorthSouth()          const = 0;

	virtual  int   northIndex(double north)      const = 0;
	virtual  int   southIndex(double south)      const = 0;
//---------------------------------------------------------------

	virtual  size_t loadGridSpec(vector<long>& v)  const = 0;
	virtual  size_t getGridDefinition(vector<long>& v) const = 0;

	virtual  int    getLatitudeOffset(double lat) const = 0;
	virtual  long   getLatitudeOffset(double lat, long& current) const = 0;
	virtual  void   getOffsets(vector<int>& offsets) const = 0;

	virtual  int   truncate(int truncation)      const = 0;
	virtual  int   match(int truncation)         const;
	virtual  int   matchGaussian()               const = 0;
	virtual  void  angularChange(vector <double>& angles) const;

	virtual int lengthOfFirstLatitude() const  { return westEastNumberOfPoints(); } ;
	virtual int lengthOfLastLatitude() const   { return westEastNumberOfPoints(); } ;

	virtual void reOrderNewData(const vector<double>& data_in, vector<double>& data_out, int scMode) const = 0; 
	virtual void   weightsY(vector<double>& w) const = 0;
	void   aWeights(vector<double>& weights) const;
	
	virtual void addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const = 0;
	virtual long getIndex(int i, int j) const = 0;

	virtual void accumulatedDataWE(const double* data, unsigned long dataLength, vector<double>& newData) const = 0;

	virtual void getGridAsLatLonList(double* lats, double* lons, long* length) const = 0;

	virtual void findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex )  const = 0;

protected:

// -- Members
	Area     area_;

// -- Methods
	
	virtual void print(ostream&) const; // Change to virtual if base class	

// -- Overridden methods
	// None

// -- Class members

// -- Class methods
	// None

private:

// No copy allowed

	Grid(const Grid&);
	Grid& operator=(const Grid&);


// -- Friends

	friend ostream& operator<<(ostream& s,const Grid& p)
		{ p.print(s); return s; }
};

#endif

