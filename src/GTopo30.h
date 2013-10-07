/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef GTopo30_H
#define GTopo30_H

#ifndef Lsm_H
#include "Lsm.h"
#endif

#include <vector>

// Forward declarations
class GTopo30Dem;


class GTopo30 : public Lsm {
public:

// -- Contructors

	GTopo30(Input* file);

// -- Destructor

	~GTopo30(); // Change to virtual if base class



private:

// No copy allowed

	GTopo30(const GTopo30&);
	GTopo30& operator=(const GTopo30&);

// -- Members

	vector<GTopo30Dem*> dems_;
	string  directoryOfPredefined_;

// Overriden methods
	double seaPoint(double latitude,double longitude) const;
	bool seaPointBool(double latitude,double longitude) const;

	bool  isAvailablePredefinedLsm();
	virtual	string  directoryOfPredefined() const { return directoryOfPredefined_; }

// Methods
	long  value(double latitude,double longitude)    const;


// -- Friends

	//friend ostream& operator<<(ostream& s,const GTopo30& p)
	//	{ p.print(s); return s; }

};

#endif
