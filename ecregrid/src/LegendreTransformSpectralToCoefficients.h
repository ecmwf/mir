/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LegendreTransformSpectralToCoefficients_H
#define LegendreTransformSpectralToCoefficients_H

#ifndef LegendreTransform_H
#include "LegendreTransform.h"
#endif

#include <vector>
#include <complex>
typedef complex<double> comp;

class LegendreTransformSpectralToCoefficients : public LegendreTransform {
public:

// -- Exceptions
	// None

// -- Contructors

	LegendreTransformSpectralToCoefficients(const ref_counted_ptr<const LegendrePolynomials>& poly, int truncation,const vector<double>& lats, bool wind, double north, double south);

// -- Destructor

	~LegendreTransformSpectralToCoefficients(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods
	// None

// -- Overridden methods
	int transform(mapLats& mapped, const vector<comp>& dataComplex, int northIndex, int southIndex) const ;
	int transform(double* coeff, const double* data, int latNumber) const;

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

	LegendreTransformSpectralToCoefficients(const LegendreTransformSpectralToCoefficients&);
	LegendreTransformSpectralToCoefficients& operator=(const LegendreTransformSpectralToCoefficients&);

// -- Members
	double north_;
	double south_;
	bool   wind_;

// -- Methods
	comp firstHarmonic(const vector<comp>& dataComplex, bool pole) const;
	int  uvAtPole(mapLats& mappedCoeff,const vector<comp>& dataComplex) const;

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	friend ostream& operator<<(ostream& s,const LegendreTransformSpectralToCoefficients& p)
		{ p.print(s); return s; }

};

#endif
