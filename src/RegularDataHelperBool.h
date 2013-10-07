/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef RegularDataHelperBool_H
#define RegularDataHelperBool_H

// namespace src;

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#ifndef Exception_H
#include "Exception.h"
#endif

template <class T>

class RegularDataHelperBool {
public:

// -- Contructors

	RegularDataHelperBool(const vector<T>& values, int scMode, int nlat, int nlon, long npts) : values_(values), scanningMode_(scMode), nlat_(nlat), nlon_(nlon) {}

// -- Destructor
	~RegularDataHelperBool() { } 


	const bool operator()(int i,int j) const { 

		if(values_.size() == 0)
			return false;

		ASSERT(i >= 0);

		int jj;

		if(j == -1)
			return false;
		if(scanningMode_ == 1)
			jj = j;
		else if(scanningMode_ == 2)
			jj = nlat_ - 1 - j;
		else
			throw NotImplementedFeature("RegularDataHelperBool - scanning mode not available ");

//		cout << "RegularDataHelperBool::operator() index " << i + jj * nlon_ << " scMode " << scanningMode_ << endl;
        long t = i + jj * nlon_;

        if(t >= npts_){
            throw OutOfRange(t,npts_);
        }

		if(values_[t] > 0.5)
			return true;
		return false;
	}

private:

// No copy allowed

	RegularDataHelperBool(const RegularDataHelperBool&);
	RegularDataHelperBool& operator=(const RegularDataHelperBool&);

// -- Members
//
// We assume object referred to exists for the lifetime of this helper
	const vector<T>& values_;
	int scanningMode_;
	int nlat_;
	int nlon_;
	long npts_;


};

#endif
