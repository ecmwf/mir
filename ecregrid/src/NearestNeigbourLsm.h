#ifndef NEARESTNEIGBOURLSM_H
#define NEARESTNEIGBOURLSM_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "InterpolatorLsm.h"

class NearestNeigbourLsm : public InterpolatorLsm {
  public:

    NearestNeigbourLsm(const Grid& input, const Grid& output, const string& lsmMethod);
    NearestNeigbourLsm(int n, const Grid& input, const Grid& output, const string& lsmMethod);

    ~NearestNeigbourLsm();

// Overriden methods
    virtual double interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const;

    // provide access to calculation algorithm where lsm data is known
    static double calculateInterpolatedValue(const Point& where, const vector<FieldPoint>& nearests, const vector<double>& inLsmData, const vector<double>& outLsmData);

  protected:
    void print(ostream&) const;

  private:

};
#endif
