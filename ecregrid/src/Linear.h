/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Linear_H
#define Linear_H

#ifndef Interpolator_H
#include "Interpolator.h"
#endif


class Linear : public Interpolator {
  public:

// -- Contructors
    Linear();
    Linear(bool w, bool a, double nPole, double sPole);

// -- Destructor
    ~Linear();

// -- Overridden methods
    void interpolationWeights(const Point& where, const vector<FieldPoint>& nearests, vector<double>& weights) const;
    virtual double interpolatedValue(const Point& point, const vector<FieldPoint>& nearests) const;

  protected:
    void print(ostream&) const;

  private:


// -- Methods
    double alongParalel(const Point& where, const FieldPoint& up, const FieldPoint& down) const;
    double alongMeridian(const Point& where, const FieldPoint& up, const FieldPoint& down) const;

    void parallelWeights(const Point& where, const FieldPoint& left, const FieldPoint& right, double& left_weight, double& right_weight) const;
    void meridianWeights(const Point& where, const FieldPoint& left, const FieldPoint& right, double& left_weight, double& right_weight) const;
};

#endif
