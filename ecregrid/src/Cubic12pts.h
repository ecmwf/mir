/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Cubic12pts_H
#define Cubic12pts_H

#ifndef Interpolator_H
#include "Interpolator.h"
#endif

//

class Cubic12pts : public Interpolator {
  public:

// -- Exceptions
    // None

// -- Contructors

    Cubic12pts();
    Cubic12pts(bool w, bool a, double nPole, double sPole);

// -- Destructor

    virtual ~Cubic12pts(); // Change to virtual if base class

  protected:
    void print(ostream&) const;
// -- Overridden methods
    virtual void interpolationWeights(const Point& where, const vector<FieldPoint>& nearests, vector<double>& weights) const;

    virtual double interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const;

  private:

};

#endif
