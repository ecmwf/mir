#ifndef BiLinearLsm_H
#define BiLinearLsm_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "InterpolatorLsm.h"

class BiLinearLsm : public InterpolatorLsm {
  public:

// -- Contructors
    BiLinearLsm(const Grid& input, const Grid& output, const string& lsmMethod);
    BiLinearLsm(bool w, bool a, double nPole, double sPole, const Grid& input, const Grid& output, const string& lsmMethod);

// -- Destructor
    virtual ~BiLinearLsm();

// -- Overridden methods
    virtual double interpolatedValue(const Point& point, const vector<FieldPoint>& nearests) const;

  protected:
    void print(ostream&) const;

  private:

// Methods
    virtual double unnormalisedWeight(const Point& p, const FieldPoint& oposite) const {
        return fabs(oposite.latitude() - p.latitude()) * fabs(oposite.longitude() - p.longitude());
    }
};

#endif
