/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Interpolator_H
#define Interpolator_H

// namespace src;

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

// Forward declarations
class Point;
class GridField;
class Grid;
class Lsm;
class FieldIdentity;
class FieldFeatures;
class GridContext;
class DerivedSubgridParameters;
//class FieldPoint;

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif
//

class Interpolator {
  public:

// -- Contructors
    Interpolator();
    Interpolator(int n);
    Interpolator(bool w, bool a, double nPole, double sPole, int n);

// -- Destructor
    virtual ~Interpolator();

// -- Methods

    virtual void  interpolate(const GridField& input, const Grid& output, vector<double>& values) const;

    virtual void interpolate(const Grid& input, const vector<double>& data, int inScMode, double inMissingValue, const vector<Point>& outputPoints, vector<double>& values) const;

    virtual double interpolate(const Grid& input, GridContext* ctx, vector<FieldPoint>& nearests, const vector<double>& data, int inScMode, double inMissingValue, const Point& outputPoint) const;

    virtual void interpolationWeights(const Point& where, const vector<FieldPoint>& nearests, vector<double>& weights) const;

    virtual double interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const = 0;

    double missingNeighbours(const Point& where, const vector<FieldPoint>& nearests, int size, double missingValue) const;
    double missingNeighbours(const Point& where, const vector<FieldPoint>& nearests, int size) const;

    void missingNeighbourWeights(const Point& where, const vector<FieldPoint>& nearests, vector<double>& weights) const;

    double useNearestNeigbour(const Point& where, const vector<FieldPoint>& nearests, int size) const;
    double useNearestNeigbour(const Point& where, const vector<FieldPoint>& nearests, int size, double missingValue) const;

    int numberOfNeighbours() const {
        return neighbour_;
    }

//	void checkConservation(const double* values, unsigned long valuesSize, const GridField& input, double missingValue) const;

    double checkConservation(const vector<double>& values, const vector<double>& weights, double missingValue) const;

    void checkValues(const vector<double>& values) const;
    void checkDumpNearest(const Point& target, long tIndex, const vector<FieldPoint>& nearests) const;

    void extrapolateOnPole(const GridField& input);

    virtual void standardDeviation(const GridField& input, const Grid& output, vector<double>& values) const;

    virtual void standardDeviation(const Grid& input, const vector<double>& data, const vector<double>& dataSquared,  int inScMode, double inMissingValue, const vector<Point>& outputPoints, vector<double>& values) const;

    virtual void derivedSubgridParameters(const GridField& input, const Grid& output, vector<double>& values, const DerivedSubgridParameters& stat) const;

    virtual void derivedSubgridParameters(const Grid& input, const vector<double>& K, const vector<double>& L, const vector<double>& M, int inScMode, double inMissingValue, const vector<Point>& outputPoints, vector<double>& values, const DerivedSubgridParameters& stat) const;

    static bool comparer(const pair<unsigned int,double>& a, const pair<unsigned int,double>& b);
  protected:
// -- Methods
    long neighboursNeeded(double nsIn, double nsOut) const;
    long ratio(double nsIn, double nsOut) const;
    virtual void print(ostream&) const;

// Members
    int    neighbour_;
    bool   extrapolateLinearOnPole_;
    bool   extrapolateAverageOnPole_;
    double averageForNorthPole_;
    double averageForSouthPole_;

  private:

// -- Friends
    friend ostream& operator<<(ostream& s,const Interpolator& p) {
        p.print(s);
        return s;
    }
};


#endif

