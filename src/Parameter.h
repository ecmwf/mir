/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Parameter_H
#define Parameter_H

// namespace lib;

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

// 

class Parameter {
public:

// -- Exceptions
	// None

// -- Contructors

	Parameter(int param, int table, const string& levType);
	Parameter();
	Parameter(const Parameter&);

// -- Destructor

	~Parameter(); // Change to virtual if base class

// -- Convertors
	// None


// -- Methods
	int  number()       const { return number_; }	
	int  table()        const { return table_; }	
//	int  marsParam()    const { return number_ * 1000 + table_; }	
	int  marsParam()    const;
	bool lsm()          const { return lsm_; }	
	bool conservation() const { return conservation_; }	
	bool wind()         const { return wind_; }	
	bool nearest()      const { return nearest_; }	

	int  vertical()     const { return vertical_; }	

	void peculiarParameters();

	Parameter& operator=(const Parameter&);

// -- Overridden methods
	// None


protected:

// -- Members
	// None

// -- Methods
	
	 void print(ostream&) const; // Change to virtual if base class	

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:


// -- Members
	// None

// -- Methods
	// None

// -- Overridden methods
	// None

// -- Class members
	int  number_;
	int  table_;
	bool lsm_;
	bool conservation_;
	bool nearest_;
	bool wind_;
	int  vertical_;


// -- Class methods
	// None

// -- Friends

	friend ostream& operator<<(ostream& s,const Parameter& p)
		{ p.print(s); return s; }

};

#endif
