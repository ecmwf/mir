/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Regular_H
#define Regular_H

#ifndef GridType_H
#include "GridType.h"
#endif

#ifndef Exception_H
#include "Exception.h"
#endif


// Forward declarations
class Rotation;
class Area;

class Regular : public GridType {
  public:

// -- Exceptions
    // None

// -- Contructors

    explicit Regular(const Grid* grid);
    explicit Regular(const Grid* grid, long nptsNS, long nptsWE);

// -- Destructor

    virtual ~Regular(); // Change to virtual if base class


// -- Operators
    long operator()(int i,int j) const;


    void nearest4(GridContext* ctx,const Point& where, vector<Point>& result)  const;

// -- Overridden methods



    void    getOffsets(vector<int>& offsets) const;
    void accumulatedDataWestEast(const double* data, unsigned long dataLength, vector<double>& newData) const;
    virtual void reOrderData(const vector<double>& data_in, vector<double>& data_out, int scMode) const;

    void    nearestPts(GridContext*,const Point& where, vector<FieldPoint>& nearests, const vector<double>& data, int scMode, int howMany) const;

    double conserving(GridContext* ctx, const Point& where, const vector<double>& data, int scMode, double missingValue, double incNs, double incWe)  const;

    void nearestIndexed(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const;

//-------------------------------------------
    void    generateGrid(vector<Point>& llgrid)  const;
    void    generateUnrotatedGrid(vector<Point>& llgrid, const Rotation& rot)  const;
    void    generateRotatedGrid(vector<Point>& llgrid, const Rotation& rot)  const;
    void    generateGridAsLatLonList(double* lats, double* lons, long* length)  const;
//-------------------------------------------

    int     getLatitudeOffset(double lat) const;
    long    getLatitudeOffset(double lat, long& current) const;

// -- Methods
    void cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const;
    double fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const;

    double averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double incWe)  const;
    double averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double outWEincrement)  const;

    unsigned long calculatedNumberOfPts() const {
        return northSouthNumberOfPoints_ * westEastNumberOfPoints_;
    } ;
    unsigned long calculatedNumberOfPoints() const {
        return calculatedNumberOfPoints_;
    } ;

    int northSouthNumberOfPts() const {
        return northSouthNumberOfPoints_;
    }
    int westEastNumberOfPts()   const {
        return westEastNumberOfPoints_;
    }

    virtual GridContext* getGridContext() const;

    size_t getGridDefinition(vector<long>& v) const;

    long getIndex(int i, int j) const {
        return i + j * westEastNumberOfPoints_;
    }

    long getIndexRotatedGlobal(int i, int j) const {
        return i + j * (westEastNumberOfPoints_ + 1);
    }

    long get1dIndex(int i, int j, int scanMode) const {
        ASSERT(i >= 0);
        ASSERT(j >= 0);
        int jj;
        if(scanMode == 1)
            jj = j;
        else if(scanMode == 2)
            jj = northSouthNumberOfPoints_ - 1 - j;
        else
            throw NotImplementedFeature("RegularDataHelper - scanning mode not available ");
//      cout << "Regular::get1dIndex " << i + jj * westEastNumberOfPoints_ << " scanMode " << scanMode << endl;
        long t = i + jj * westEastNumberOfPoints_;

        if(t >= (long)calculatedNumberOfPoints_) {
            throw OutOfRange(t,calculatedNumberOfPoints_);
        }
        return t;
    }

    void addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const;

    void findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex )  const;

  protected:

// -- Members
    vector<double> lats_;
    vector<double> lons_;

// -- Methods

    virtual void print(ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    Regular(const Regular&);
    Regular& operator=(const Regular&);

// -- Members
    int northSouthNumberOfPoints_;
    int westEastNumberOfPoints_;
    unsigned long calculatedNumberOfPoints_;
    long   lastLatitudeIndex_;
    long   lastLongitudeIndex_;
    double northSouthIncrement_;
    double westEastIncrement_;

// ssp temporary to support Emos simulation
    bool   emosRotateGlobal_;

// -- Methods
    void generateLongitudes(int numberOfPoints, double increment, double west, double east, vector<double>& longitudes) const;
    Area cell(GridContext* ctx, bool flux, const Point& where, const vector<double>& outLats, double outWEincrement, long& startJ, long& endJ, long& startI, long& endI, long& startII, long& endII) const;

// -- Friends

    //friend ostream& operator<<(ostream& s,const Regular& p)
    //	{ p.print(s); return s; }

};

#endif
