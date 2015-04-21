/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LinearFit3TimesBitmap_H
#define LinearFit3TimesBitmap_H

#ifndef LinearFit3Times_H
#include "LinearFit3Times.h"
#endif


class LinearFit3TimesBitmap : public LinearFit3Times {
  public:

// -- Contructors
    LinearFit3TimesBitmap(double missingValue);
    LinearFit3TimesBitmap(bool w, bool a, double nPole, double sPole, double missingValue);

// -- Destructor
    ~LinearFit3TimesBitmap();

  protected:
    void print(ostream&) const;

  private:

// -- Overridden methods
    double interpolatedValue(const Point& point, const vector<FieldPoint>& nearests) const;

    double missingValue_;

};

#endif
