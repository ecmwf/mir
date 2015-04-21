/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef RegularDataIndexHelper_H
#define RegularDataIndexHelper_H

// namespace src;

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#ifndef Exception_H
#include "Exception.h"
#endif


class RegularDataIndexHelper {
  public:

// -- Contructors

    RegularDataIndexHelper(int scMode, int nlat, int nlon) : scanningMode_(scMode), nlat_(nlat), nlon_(nlon) {}

// -- Destructor

    ~RegularDataIndexHelper() { }


    long operator()(int i,int j) const {


        ASSERT(i >= 0 && j >= 0);

        if(scanningMode_ == 1)
            return i + j * nlon_;
        else if(scanningMode_ == 2)
            return i + (nlat_ - 1 - j) * nlon_;

        throw NotImplementedFeature("RegularDataHelper - scanning mode not available ");

//		cout << "RegularDataIndexHelper::operator() index " << i + jj * nlon_ << " scMode " << scanningMode_ << endl;

        return 0;
    }

  private:

// No copy allowed

    RegularDataIndexHelper(const RegularDataIndexHelper&);
    RegularDataIndexHelper& operator=(const RegularDataIndexHelper&);

// -- Members
    int scanningMode_;
    int nlat_;
    int nlon_;
};

#endif
