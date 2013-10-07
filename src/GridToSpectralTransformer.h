/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef GridToSpectralTransformer_H
#define GridToSpectralTransformer_H

#ifndef   Transformer_H
#include "Transformer.h"
#endif

// Forward declarations

// 

class GridToSpectralTransformer : public Transformer {
public:

// -- Exceptions
	// None

// -- Contructors

	GridToSpectralTransformer(const string& legendrePolynomialsMethod, int fftMax);

// -- Destructor

	~GridToSpectralTransformer(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods
	// None

// -- Overridden methods
	Field* transform(const Field& in, const Field& out) const;
	Wind*  transformVector(const Field& inU, const Field& inV, const Field& req) const;

// -- Class members
	// None

// -- Class methods
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

// No copy allowed

	GridToSpectralTransformer(const GridToSpectralTransformer&);
	GridToSpectralTransformer& operator=(const GridToSpectralTransformer&);

// -- Members
	string legendrePolynomialsMethod_;
	int    fftMaxBlockSize_;

// -- Methods
	// None

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	friend ostream& operator<<(ostream& s,const GridToSpectralTransformer& p)
		{ p.print(s); return s; }

};

#endif
