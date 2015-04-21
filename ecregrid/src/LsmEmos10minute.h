/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LsmEmos10minute_H
#define LsmEmos10minute_H

#ifndef Lsm_H
#include "Lsm.h"
#endif


class LsmEmos10minute : public Lsm {

  public:
// -- Contructors

    LsmEmos10minute( Input* file);

// -- Destructor

    virtual ~LsmEmos10minute(); // Change to virtual if base class


  private:

// No copy allowed

    LsmEmos10minute(const LsmEmos10minute&);
    LsmEmos10minute& operator=(const LsmEmos10minute&);

// -- Members

    vector<double> lsm10min_;
    string  directoryOfPredefined_;

// Overriden methods
    double  seaPoint(double latitude,double longitude) const;
    bool  seaPointBool(double latitude,double longitude) const;

    bool  isAvailablePredefinedLsm();

// Methods
    string  directoryOfPredefined() const {
        return directoryOfPredefined_;
    }

};

#endif
