/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef ReducedDataHelperBool_H
#define ReducedDataHelperBool_H

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#ifndef Exception_H
#include "Exception.h"
#endif


template <class T>

class ReducedDataHelperBool {
  public:

// -- Contructors

    ReducedDataHelperBool(const vector<T>& values, int scMode, const vector<long>&  offsets, int nlat):
        values_(values),
        scanningMode_(scMode),
        offsets_(offsets),
        nlat_(nlat) {
    }

// -- Destructor

    ~ReducedDataHelperBool() {
//		if(DEBUG)
//			cout << "~ReducedDataHelperBool Yes: " << yes_ << ", No: " << no_ << endl;
    } // Change to virtual if base class


    const bool operator()(int i, int j) const {

        if(values_.size() == 0)
            return false;


        int jj;

        if(j == -1)
            return false;

        ASSERT(i >= 0);

        if(scanningMode_ == 1)
            jj = j;
        else if(scanningMode_ == 2)
            jj = nlat_ - 1 - j;
        else
            throw NotImplementedFeature("ReducedDataHelperBool - scanning mode not available ");

//		cout << "ReducedDataHelperBool offset_ " << offset_ << " i " << i << " value " << data[offset_ + i ] << endl;

        if(values_[offsets_[jj] + i ] >= 0.5)
            return true;

        return false;
    }



  private:

// No copy allowed

    ReducedDataHelperBool(const ReducedDataHelperBool&);
    ReducedDataHelperBool& operator=(const ReducedDataHelperBool&);

// -- Members
// We assume the object referred to exists for the lifetime of this helper
    const vector<T>& values_;

    int          scanningMode_;
    long         lastJ_;
    long         offset_;
    int          nlat_;
    const        vector<long>&  offsets_;

    long yes_;
    long no_;

};

#endif
