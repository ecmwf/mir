// File Seconds.h
// Baudouin Raoult - ECMWF Jul 96

#ifndef Seconds_H
#define Seconds_H
 
#ifndef machine_H
#include "machine.h"
#endif

class Bless;

class Seconds {
public:

// -- Contructors

	Seconds(double);
	Seconds(const struct timeval&);

// -- Operators

	operator string() const;
	operator double() const { return seconds_; }

	friend ostream& operator<<(ostream&,const Seconds&);

private:


// -- Members

	double seconds_;

};

#endif
