#ifndef AverageLsm_H
#define AverageLsm_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "InterpolatorLsm.h"

class AverageLsm : public InterpolatorLsm {
  public:

    // -- Contructors
    AverageLsm(const Grid &input, const Grid &output, const string &lsmMethod);
    AverageLsm(int n, const Grid &input, const Grid &output, const string &lsmMethod);

    // -- Destructor
    ~AverageLsm();

  protected:
    void print(ostream &) const;

  private:

    // -- Overridden methods
    double interpolatedValue(const Point &point, const vector<FieldPoint> &nearests) const;
};
#endif
