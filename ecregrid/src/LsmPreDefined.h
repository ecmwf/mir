/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LsmPreDefined_H
#define LsmPreDefined_H

#ifndef Lsm_H
#include "Lsm.h"
#endif


class LsmPreDefined : public Lsm {

  public:
// -- Contructors

    LsmPreDefined(Input* input);
    LsmPreDefined(Input* input, const string& predefined);

// -- Destructor

    virtual ~LsmPreDefined(); // Change to virtual if base class


// Overriden methods
    ref_counted_ptr< const vector<double> > getLsmValuesDouble(const Grid& gridSpec);
    void   getLsmValues(const Grid& grid, vector<bool>& generatedLsm) ;

  private:

// No copy allowed

    LsmPreDefined(const LsmPreDefined&);
    LsmPreDefined& operator=(const LsmPreDefined&);

// Overriden methods
    double  seaPoint(double latitude,double longitude) const;
    bool  seaPointBool(double latitude,double longitude) const;

    bool  isAvailablePredefinedLsm();
    string  directoryOfPredefined() const {
        return directoryOfPredefined_;
    }

    string directoryOfPredefined_;



};

#endif
