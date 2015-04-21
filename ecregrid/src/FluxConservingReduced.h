#ifndef FLUXCONSERVINGREDUCED_H
#define FLUXCONSERVINGREDUCED_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Interpolator.h"

class Grid;
class Area;

class FluxConservingReduced : public Interpolator {
  public:

    // -- Contructors
    FluxConservingReduced(const Grid &in, const Grid &out);

    // -- Destructor
    ~FluxConservingReduced();

  protected:
    void print(ostream &) const;

  private:
    vector<Area>   cellArea_;
    vector<double> cellAreaSize_;

    int northSouthNumberOfPoints_;
    double northSouthIncrement_;
    bool westEastGlobal_;
    double east_;
    double west_;
    vector<long>  gridSpec_;
    size_t        sizeSpec_;

    // -- Methods
    double westEastIncrement(long westEastNumPts) const {
        if (westEastGlobal_)
            return 360.0 / westEastNumPts;
        return (east_ - west_) / (westEastNumPts - 1);
    }
    // -- Overridden methods
    virtual void interpolate(const Grid &input, const double *data, int inScMode, double missingValue, const vector<Point> &outputPoints, double *values, unsigned long valuesSize) const;

    double interpolatedValue(const Point &point, const vector<FieldPoint> &nearests ) const;

    void standardDeviation(const Grid &input, const double *data, const double *dataSquared,  int inScMode, double missingValue, const vector<Point> &outputPoints, double *values, unsigned long valuesSize) const;

    void derivedSubgridParameters(const Grid &input, const double *K, const double *L, const double *M, int inScMode, double missingValue, const vector<Point> &outputPoints, double *values, unsigned long valuesSize, const DerivedSubgridParameters &derived) const;
};
#endif
