#ifndef FLUXCONSERVINGREGULAR_H
#define FLUXCONSERVINGREGULAR_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Interpolator.h"

class Grid;
class Area;

class FluxConservingRegular : public Interpolator {
  public:

    // -- Contructors
    FluxConservingRegular(const Grid &in, const Grid &out);

    // -- Destructor
    ~FluxConservingRegular();

  protected:
    void print(ostream &) const;

  private:
    vector<Area> cellArea_;
    vector<double> cellAreaSize_;

    int northSouthNumberOfPoints_;
    int westEastNumberOfPoints_;

    double northSouthIncrement_;
    double westEastIncrement_;

    // -- Methods
    // -- Overridden methods
    virtual void interpolate(const Grid &input, const double *data, int inScMode, double missingValue, const vector<Point> &outputPoints, double *values, unsigned long valuesSize) const;

    double interpolatedValue(const Point &point, const vector<FieldPoint> &nearests) const;

    void standardDeviation(const Grid &input, const double *data, const double *dataSquared,  int inScMode, double missingValue, const vector<Point> &outputPoints, double *values, unsigned long valuesSize) const;

    void derivedSubgridParameters(const Grid &input, const double *K, const double *L, const double *M, int inScMode, double missingValue, const vector<Point> &outputPoints, double *values, unsigned long valuesSize, const DerivedSubgridParameters &derived) const;
};
#endif
