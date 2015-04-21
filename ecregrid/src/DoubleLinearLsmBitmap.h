/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef DoubleLinearLsmBitmap_H
#define DoubleLinearLsmBitmap_H

#ifndef DoubleLinearLsm_H
#include "DoubleLinearLsm.h"
#endif

//

class DoubleLinearLsmBitmap : public DoubleLinearLsm {
  public:

// -- Contructors

    DoubleLinearLsmBitmap(const Grid& input, const Grid& output, const string& lsmMethod, double missingValue);
    DoubleLinearLsmBitmap(bool w, bool a, double nPole, double sPole, const Grid& input, const Grid& output, const string& lsmMethod, double missingValue);

// -- Overridden methods
    virtual double interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const;

// -- Destructor

    ~DoubleLinearLsmBitmap(); // Change to virtual if base class

  protected:
    void print(ostream&) const;

  private:
    double missingValue_;

};

#endif
