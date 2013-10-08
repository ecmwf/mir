/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef GTopo30Dem_H
#define GTopo30Dem_H

#include <string>
using namespace std;

// namespace gtopo;

// Headers
// #ifndef   machine_H
// #include "machine.h"
// #endif

// Forward declarations

// class ostream;

// 

class GTopo30Dem {
public:

// -- Exceptions
	// None

// -- Contructors

	GTopo30Dem(const string& path,const string& name,double minLatitude,double maxLatitude,double minLongitude,double maxLongitude);

// -- Destructor

	~GTopo30Dem(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods
	
	bool contains(double latitude,double longitude) const
		{ return latitude >= minLatitude_ && latitude <= maxLatitude_ && 
		         longitude >= minLongitude_ && longitude <= maxLongitude_; 
     	 }

	long value(double latitude,double longitude) const;
		

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

protected:

// -- Members
	// None

// -- Methods
	
	// void print(ostream&) const; // Change to virtual if base class	

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	GTopo30Dem(const GTopo30Dem&);
	GTopo30Dem& operator=(const GTopo30Dem&);

// -- Members

	string path_;
	string name_;

	int fd_;


	double minLatitude_;
	double maxLatitude_;
	double minLongitude_;
	double maxLongitude_;


	double ulxmap_;
	double ulymap_;

	double xdim_;
	double ydim_;

	long ncols_;


// -- Methods

	void open();

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const GTopo30Dem& p)
	//	{ p.print(s); return s; }

};

#endif
