/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Timer_H
#define Timer_H

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

#include <iostream>
#include <string>

using std::string;
using std::ostream;


// This stack object prints the elapse time between the call to
// its contructor and the call to its destructor

class Timer {
public:

// -- Contructors

	Timer(const std::string& name, std::ostream& = std::cout);

// -- Destructor

	~Timer();

// -- Methods

	double elapsed();

private:

// No copy allowed

	Timer(const Timer&);
	Timer& operator=(const Timer&);

// -- Members
	
	string         name_;
	struct timeval start_;
	clock_t        cpu_;
	ostream&       out_;

// -- Methods
	
	ostream& put(ostream&,double);

};

timeval operator-(const timeval&,const timeval&);


#endif
