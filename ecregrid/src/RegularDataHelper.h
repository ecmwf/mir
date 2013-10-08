/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef RegularDataHelper_H
#define RegularDataHelper_H

// namespace src;

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#ifndef Exception_H
#include "Exception.h"
#endif

template <class T>

class RegularDataHelper {
public:

// -- Contructors

	RegularDataHelper(const vector<T>& values, int scMode, double missingValue, int nlat, int nlon, long npts) : values_(values), scanningMode_(scMode), missingValue_(missingValue), nlat_(nlat), nlon_(nlon), npts_(npts) {}

// -- Destructor

	~RegularDataHelper() { } 


	const T operator()(int i,int j) const { 

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

//		cout << "RegularDataHelper::operator() index " << i + jj * nlon_ << " scMode " << scanningMode_ << endl;
		long t = i + jj * nlon_;

		if(t >= npts_){
			throw OutOfRange(t,npts_);
		}

		return values_[t]; 
	}

private:

// No copy allowed

	RegularDataHelper(const RegularDataHelper&);
	RegularDataHelper& operator=(const RegularDataHelper&);

// -- Members
//  We assume the object referred to exists for the lifetime of this helper
	const vector<T>& values_;
	int scanningMode_;
	double missingValue_;
	int nlat_;
	int nlon_;
	long npts_;


};

#endif
