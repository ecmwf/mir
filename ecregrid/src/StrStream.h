/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef StrStream_H
#define StrStream_H

#ifndef   machine_H
#include "machine.h"
#endif

#ifdef HAS_STRINGSTREAM

class StrStream : public ostringstream {
public:
	operator string() { return str(); }
	static ostream&  ends(ostream& os)  { return os; }
};

#else

#include <strstream>

// ostrstream will cahnge to stringstream when
// it exist

class StrStream : public ostrstream {
public:

	enum { size = 1024 }; // Should be enough

// -- Contructors

	// Use this to prevent a warning because buffer_ 
	// is yet not initalised. (It does not have to be)

	StrStream() : ostrstream(this->buffer_,size) {}

// -- Convertors

	operator const char*()      { return buffer_ ; }

	static ostream&  ends(ostream& os)  { os << std::ends ; return os; }

private:

// No copy allowed

	StrStream(const StrStream&);
	StrStream& operator=(const StrStream&);

// -- Members

	char buffer_[size]; 
};

#endif


#endif
