/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef FieldIdentity_H
#define FieldIdentity_H

// namespace src;

// Headers
#ifndef   machine_H
#include "machine.h"
#endif


// Forward declarations

// 

struct FieldIdentity {

// -- Contructors

	FieldIdentity();
	FieldIdentity(const string& composedName);
	FieldIdentity(bool gridOrSpectral, const string& gridType, bool reduced, bool stretched, bool rotated );

	FieldIdentity(const FieldIdentity&);

	void isAvailable();

// -- Destructor

	~FieldIdentity(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	FieldIdentity& operator=(const FieldIdentity&);
	bool operator==(const FieldIdentity&) const;
	bool operator|(const FieldIdentity& id) const;

	string gridType() const { return gridType_; }
	void   setFromTypeOfGrid(const string& name);

// -- Members
    string  composedName_;
    bool    gridOrSpectral_;
    string  gridType_;
    bool    reduced_;
    bool    stretched_;
    bool    rotated_;

protected:

// -- Methods
	 void print(ostream&) const; // Change to virtual if base class	

private:


// -- Friends

	friend ostream& operator<<(ostream& s,const FieldIdentity& p)
		{ p.print(s); return s; }

};

#endif
