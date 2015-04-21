/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Projection_H
#define Projection_H

#ifndef Grid_H
#include "Grid.h"
#endif

// Forward declarations

class Projection : public Grid {
  public:

// -- Contructors
    Projection();
    Projection( const vector<Point>& llgrid, double north, double west, double south, double east);

// -- Destructor
    virtual ~Projection(); // Change to virtual if base class

// Methods
    double calculateNortSouthIncrement(double north, double south, long npts) const;
//	void  global();

  protected:

// Overriden methods
    void print(ostream&) const;

// Members
    vector<Point> points_;

// -- Methods

  private:

// No copy allowed
    Projection(const Projection&);
    Projection& operator=(const Projection&);


// Overriden methods

    Grid*    getGlobalGrid()            const ;

    void nearest4pts(GridContext* ctx,const Point& where, vector<Point>& result) const;
    void nearestPoints(GridContext*,const Point& where, vector<FieldPoint>& nearests, const vector<double>& data, int scMode, int npts) const;

    double conserving(GridContext* ctx, const Point& where, const vector<double>& data, int scMode, double missingValue, double incNs, double incWe)  const;

    void cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const;
    double fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const;

    double averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double incWe)  const;
    double averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data,const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double incWe)  const;
    void nearestsByIndex(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const;

    void     generateGrid1D(vector<Point>& p)    const;
    void  generateGrid1D(vector<Point>& ll, const Grid& grid) const;

    unsigned long calculatedNumberOfPoints() const ;


    size_t  getGridDefinition(vector<long>& v) const;

    string   composedName()  const {
        return "polar_stereographic";
    }

    int    getLatitudeOffset(double lat)         const;
    long   getLatitudeOffset(double lat, long& current) const;
    void   getOffsets(vector<int>& offsets)      const;


    GridContext* getGridContext() const;

    virtual void reOrderNewData(const vector<double>& data_in, vector<double>& data_out, int scMode) const;

    void addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const;
    long getIndex(int i, int j) const;
    void accumulatedDataWE(const double* data, unsigned long dataLength, vector<double>& newData) const;
//=======================================================================
    vector<double> latitudes() const;
    virtual void latitudes(vector<double>& lats) const;
    void northHemisphereLatitudes(vector<double>& latitudes) const;


    int     poleEquatorNumberOfPoints() const;
    virtual string   gridType()         const {
        return "projection";
    }

    string   coeffInfo()  const;

    Area  adjustArea(const Area& area, bool globalWE, bool globalNS) const;
    void  adjustAreaWestEastMars( double& west, double& east, double increment) const;
    void  setGlobalNorthSouth(double& north, double& south) const;

    virtual bool  isGlobalNorthSouth()       const ;

    int   northIndex(double north)   const;
    int   southIndex(double south)   const;
    size_t  loadGridSpec(vector<long>& rgSpec) const;

    int    truncate(int truncation)      const;
    int    matchGaussian()               const;

    void  weightsY(vector<double>& w) const;

    Area  setGlobalArea()               const;
    Area  setGlobalArea(double west)    const;
    Area  fitToGlobalArea(double west)  const;
    bool  isGlobalWestEast()            const {
        return true;
    }

    void getGridAsLatLonList(double* lats, double* lons, long* length) const;

    void findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex ) const;

// -- Methods

    void  dump()                   const ;

};

#endif
