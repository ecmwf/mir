/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef ListOfPoints_H
#define ListOfPoints_H

// namespace lib;

// Headers
#ifndef Grid_H
#include "Grid.h"
#endif

#include <vector>

// Forward declarations
class Point;
// 

/*! \class ListOfPoints
    \brief List Of Points
	
	Free style List of Points
*/

class ListOfPoints : public Grid {
public:

// -- Contructors

	ListOfPoints(const vector<Point>& points);

	ListOfPoints(const vector<Point>& points, double tolerance);

	ListOfPoints(const Area& area, const vector<Point>& points, double tolerance);

// -- Destructor

	~ListOfPoints(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods
	void generateList(vector<Point>& points) const;

// -- Overridden methods
	Area  setGlobalArea(double west)          const;
	Area  fitToGlobalArea(double west)        const;


// -- Class members
	// None

// -- Class methods
	// None

protected:

// -- Members
	// None

// -- Methods
	
	 void print(ostream&) const; // Change to virtual if base class	

// -- Overridden methods
	void dump2file(const string& name, const double* data) const;
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	ListOfPoints(const ListOfPoints&);
	ListOfPoints& operator=(const ListOfPoints&);

// -- Members
	vector<Point> points_;
	double        tolerance_;

// -- Overridden methods
//	Interpolator*   getPreferredInterpolator()          const;
	void generateGrid1D(vector<Point>& p) const;
	void generateGrid1D(vector<Point>& ll, const Grid& grid)    const ;

	Grid*    newListGrid(const Grid& other) const;
	Grid*    newGrid(const Grid& other) const;
	Grid* newGrid(const Area& other)    const;
	Grid*    getGlobalGrid()            const;

	string   lsmFileType() const { return "ascii"; }

	GridContext* getGridContext() const;

	void nearest4pts(GridContext* ctx,const Point& where, vector<Point>& result) const;
	void  nearestPoints(GridContext*,const Point& where, vector<FieldPoint>& result, const vector<double>& data, int scMode, int howMany)  const;

	double conserving(GridContext* ctx, const Point& where, const vector<double>& data, int scMode, double missingValue, double incNs, double incWe)  const;

	void cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const; 
	double fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const;
	double averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double incWe)  const;
	double averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double incWe)  const;

	void nearestsByIndex(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const;

	vector<double> latitudes()      const;
	void latitudes(vector<double>& lats) const;
	void    northHemisphereLatitudes(vector<double>& latitudes) const;
	double  westEastIncrement()             const;
    double  northSouthIncrement()           const;
	int     poleEquatorNumberOfPoints()     const;
	virtual string   gridType()             const { return "list"; }
	        string   coeffInfo()            const;

	Area  setGlobalArea() const;
	void  setGlobalNorthSouth(double& north, double& south) const;
	Area  adjustArea( const Area& area) const;
	Area  adjustArea(const Area& area, bool globalWE, bool globalNS) const;
	Area  adjustAreaMars( const Area& area) const;
	void  adjustAreaWestEast( double& west, double& east, double increment) const;
	void  adjustAreaWestEastMars( double& west, double& east, double increment) const;

	bool  isGlobalWestEast()         const ;
	bool  isGlobalNorthSouth()       const ;
	int westEastNumberOfPoints()   const;
	int northSouthNumberOfPoints() const;

	size_t  loadGridSpec(vector<long>& rgSpec) const;
	int   northIndex(double north)   const;
	int   southIndex(double south)   const;

	unsigned long  calculatedNumberOfPoints() const;
	size_t  getGridDefinition(vector<long>& v) const;

	string   composedName()  const { return "list"; }
	int    getLatitudeOffset(double lat) const;
	long   getLatitudeOffset(double lat, long& current) const;
	void   getOffsets(vector<int>& offsets) const;
	int    truncate(int truncation)      const;
	int    matchGaussian()               const;

	virtual bool sameAs(const RegularLatLon& other)   const  { return Grid::sameAs(other); }
	virtual bool sameAs(const RegularLatLonCellCentered& other)   const  { return Grid::sameAs(other); }
	virtual bool sameAs(const ReducedLatLon& other)   const  { return Grid::sameAs(other); }
	virtual bool sameAs(const RotatedRegularLatLon& other)   const  { return Grid::sameAs(other); }
	virtual bool sameAs(const ReducedGaussian& other) const  { return Grid::sameAs(other); }
	virtual bool sameAs(const RegularGaussian& other) const  { return Grid::sameAs(other); }
	virtual bool sameAs(const PseudoRegularGaussian& other) const  { return Grid::sameAs(other); }
	virtual bool sameAs(const PolarStereographic& other) const  { return Grid::sameAs(other); }
	virtual bool sameAs(const ListOfPoints&) const ;
	virtual bool equals(const Grid& other) const { return other.sameAs(*this); }
// -- Methods

    virtual void reOrderNewData(const vector<double>& data_in, vector<double>& data_out, int scMode) const;
	string   predefinedLsmFileName() const;
	void  weightsY(vector<double>& w) const;
	void addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const;
	long getIndex(int i, int j) const;
	void accumulatedDataWE(const double* data, unsigned long dataLength, vector<double>& newData) const;

	void getGridAsLatLonList(double* lats, double* lons, long* length) const;

	void findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex ) const;

// -- Friends

	friend ostream& operator<<(ostream& s,const ListOfPoints& p)
		{ p.print(s); return s; }

};

#endif
