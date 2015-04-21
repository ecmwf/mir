/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef BiLinearLsmBitmap_H
#define BiLinearLsmBitmap_H

#ifndef BiLinearLsm_H
#include "BiLinearLsm.h"
#endif

class BiLinearLsmBitmap : public BiLinearLsm {
  public:

// -- Contructors
    BiLinearLsmBitmap(const Grid& input, const Grid& output, const string& lsmMethod, double missingValue);
    BiLinearLsmBitmap(bool w, bool a, double nPole, double sPole, const Grid& input, const Grid& output, const string& lsmMethod, double missingValue);

// -- Destructor
    ~BiLinearLsmBitmap();

// -- Overridden methods
    virtual double interpolatedValue(const Point& point, const vector<FieldPoint>& nearests) const;

  protected:
    void print(ostream&) const;

  private:
    double missingValue_;

};

#endif
