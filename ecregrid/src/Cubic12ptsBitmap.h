/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Cubic12ptsBitmap_H
#define Cubic12ptsBitmap_H

#ifndef Cubic12pts_H
#include "Cubic12pts.h"
#endif

//

class Cubic12ptsBitmap : public Cubic12pts {
  public:

// -- Exceptions
    // None

// -- Contructors

    Cubic12ptsBitmap(double missingValue);
    Cubic12ptsBitmap(bool w, bool a, double nPole, double sPole, double missingValue);

// -- Destructor

    ~Cubic12ptsBitmap(); // Change to virtual if base class

  protected:
    void print(ostream&) const;

  private:
// -- Overridden methods
    virtual double interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const;

    double missingValue_;

};

#endif
