/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef DoubleLinearLsm_H
#define DoubleLinearLsm_H

#ifndef InterpolatorLsm_H
#include "InterpolatorLsm.h"
#endif

//

class DoubleLinearLsm : public InterpolatorLsm {
  public:

// -- Contructors

    DoubleLinearLsm(const Grid& input, const Grid& output, const string& lsmMethod);
    DoubleLinearLsm(bool w, bool a, double nPole, double sPole, const Grid& input, const Grid& output, const string& lsmMethod);

// -- Overridden methods
    virtual double interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const;

    static double calculateInterpolatedValue(const Point& where, const vector<FieldPoint>& nearests, const vector<double>& inLsmData, const vector<double>& outLsmData);
// -- Destructor

    virtual ~DoubleLinearLsm(); // Change to virtual if base class

  protected:
    void print(ostream&) const;

  private:

};

#endif
