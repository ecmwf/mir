#ifndef AverageWeightedReduced_H
#define AverageWeightedReduced_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Interpolator.h"

class Grid;

class AverageWeightedReduced : public Interpolator {
  public:

    // -- Contructors
    AverageWeightedReduced(const Grid &in, const Grid &out);

    // -- Destructor
    ~AverageWeightedReduced();

  protected:
    void print(ostream &) const;

  private:
    vector<double> weights_;
    vector<double> outLats_;

    int northSouthNumberOfPoints_;
    double northSouthIncrement_;
    bool westEastGlobal_;
    double east_;
    double west_;

    vector<long> gridSpec_;
    size_t       sizeSpec_;

    // -- Methods
    double westEastIncrement(long westEastNumPts) const {
        if (westEastGlobal_)
            return 360.0 / westEastNumPts;
        return (east_ - west_) / (westEastNumPts - 1);
    }
    // -- Overridden methods

    virtual void interpolate(const Grid &input, const vector<double> &data, int inScMode, double inMissingValue, const vector<Point> &outputPoints, vector<double> &values) const;

    virtual void  interpolate(const GridField &input, const Grid &output, vector<double> &values) const {
        return Interpolator::interpolate(input, output, values);
    }

    virtual double interpolate(const Grid &input, GridContext *ctx, vector<FieldPoint> &nearests, const vector<double> &data, int inScMode, double inMissingValue, const Point &outputPoint) const {
        return Interpolator::interpolate(input, ctx, nearests, data, inScMode, inMissingValue, outputPoint);
    }

    double interpolatedValue(const Point &point, const vector<FieldPoint> &nearests) const;

    virtual void standardDeviation(const Grid &input, const vector<double> &data, const vector<double> &dataSquared,  int inScMode, double inMissingValue, const vector<Point> &outputPoints, vector<double> &values) const;

    virtual void standardDeviation(const GridField &input, const Grid &output, vector<double> &values) const {
        return Interpolator::standardDeviation(input, output, values);
    }

    virtual void derivedSubgridParameters(const GridField &input, const Grid &output, vector<double> &values, const DerivedSubgridParameters &stat) const {
        return Interpolator::derivedSubgridParameters(input, output, values, stat);
    }

    virtual void derivedSubgridParameters(const Grid &input, const vector<double> &K, const vector<double> &L, const vector<double> &M, int inScMode, double inMissingValue, const vector<Point> &outputPoints, vector<double> &values, const DerivedSubgridParameters &stat) const;



};
#endif
