/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef SpectralToSpectralTransformer_H
#define SpectralToSpectralTransformer_H

#ifndef   Transformer_H
#include "Transformer.h"
#endif

#include <complex>
typedef complex<double> comp;

// Forward declarations
class Field;
class Wind;
class Wind;
class SpectralField;


class SpectralToSpectralTransformer : public Transformer {
public:

// -- Contructors

	SpectralToSpectralTransformer(bool conversion=true);

// -- Destructor

	~SpectralToSpectralTransformer(); // Change to virtual if base class


// -- Methods
//	Wind* vorticityDivergenceToUV(const Field& inU, const Field& inV, int trunc=0) const;
	void transform(const SpectralField& input, int outTrunc, vector<comp>& compOut) const;
	bool transform(const SpectralField& input, int outTrunc, vector<double>& compOut) const;
	void vorticityDivergenceToUV(const vector<comp>& vor, const vector<comp>& div, int toup, vector<comp>& uu, vector<comp>& vv) const;

// -- Overridden methods
	Field* transform(const Field& input, const Field& output) const;
	Wind*  transformVector(const Field& inU, const Field& inV, const Field& req) const;

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

	SpectralToSpectralTransformer(const SpectralToSpectralTransformer&);
	SpectralToSpectralTransformer& operator=(const SpectralToSpectralTransformer&);

// -- Members
	bool vdConversion_;
	// None

// -- Methods
	// None

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const SpectralToSpectralTransformer& p)
	//	{ p.print(s); return s; }

};

#endif
