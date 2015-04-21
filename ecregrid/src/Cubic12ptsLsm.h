/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Cubic12ptsLsm_H
#define Cubic12ptsLsm_H

#ifndef InterpolatorLsm_H
#include "InterpolatorLsm.h"
#endif

//

class Cubic12ptsLsm : public InterpolatorLsm {
  public:

// -- Contructors

    Cubic12ptsLsm(const Grid& input, const Grid& output, const string& lsmMethod);
    Cubic12ptsLsm(bool w, bool a, double nPole, double sPole, const Grid& input, const Grid& output, const string& lsmMethod);

// -- Destructor

    virtual ~Cubic12ptsLsm(); // Change to virtual if base class

  protected:
    void print(ostream&) const;
// -- Overridden methods
    virtual double interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const;

  private:

};

#endif
