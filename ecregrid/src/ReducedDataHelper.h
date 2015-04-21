/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef ReducedDataHelper_H
#define ReducedDataHelper_H

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#ifndef Exception_H
#include "Exception.h"
#endif


template <class T>

class ReducedDataHelper {
  public:

// -- Contructors

    ReducedDataHelper(const vector<T>& values, int scMode, double missingValue, const vector<long>&  offsets, int nlat):
        values_(values),
        scanningMode_(scMode),
        nlat_(nlat),
        offsets_(offsets),
        missingValue_(missingValue) {
    }

// -- Destructor

    ~ReducedDataHelper() {
//		if(DEBUG)
//			cout << "~ReducedDataHelper Yes: " << yes_ << ", No: " << no_ << endl;
    } // Change to virtual if base class


    const T operator()(int i, int j) const {
        if(values_.size() == 0)
            return missingValue_;

        ASSERT(i >= 0);
        ASSERT(j >= 0);

        int jj;
        if(scanningMode_ == 1)
            jj = j;
        else if(scanningMode_ == 2)
            jj = nlat_ - 1 - j;
        else
            throw NotImplementedFeature("RegularDataHelper - scanning mode not available ");

//		cout << "ReducedDataHelper offset_ " << offset_ << " i " << i << " value " << data[offset_ + i ] << endl;

        return values_[offsets_[jj] + i ];
    }



  private:

// No copy allowed

    ReducedDataHelper(const ReducedDataHelper&);
    ReducedDataHelper& operator=(const ReducedDataHelper&);

// -- Members
//
    // take a reference to the data. We assume it exists for the lifetime of
    // this helper object
    const vector<T>& values_;

    int          scanningMode_;
    long         lastJ_;
    long         offset_;
    int          nlat_;
    const        vector<long>&  offsets_;
    double       missingValue_;

    long yes_;
    long no_;

};

#endif
