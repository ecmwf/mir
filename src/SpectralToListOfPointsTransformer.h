/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef SpectralToListOfPointsTransformer_H
#define SpectralToListOfPointsTransformer_H

#ifndef   SpectralToGridTransformer_H
#include "SpectralToGridTransformer.h"
#endif

// Forward declarations
class Point;

class SpectralToListOfPointsTransformer : public SpectralToGridTransformer {
public:

// -- Exceptions
	// None

// -- Contructors

	SpectralToListOfPointsTransformer(const string& coeffMethod, int fftMax, bool auresol, bool conversion);

// -- Destructor

	~SpectralToListOfPointsTransformer(); // Change to virtual if base class

// -- Methods

// -- Overridden methods
	Field* transform(const Field& in, const Field& out) const;

protected:

// -- Members

// -- Methods
	
	void print(ostream&) const; // Change to virtual if base class	

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	SpectralToListOfPointsTransformer(const SpectralToListOfPointsTransformer&);
	SpectralToListOfPointsTransformer& operator=(const SpectralToListOfPointsTransformer&);

// -- Members

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods

// -- Friends

	friend ostream& operator<<(ostream& s,const SpectralToListOfPointsTransformer& p)
		{ p.print(s); return s; }

};

#endif
