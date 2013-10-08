/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LegendreTransformCoefficientsToSpectral_H
#define LegendreTransformCoefficientsToSpectral_H


#ifndef LegendreTransform_H
#include "LegendreTransform.h"
#endif
// 
#include <vector>

class LegendreTransformCoefficientsToSpectral : public LegendreTransform {
public:

// -- Exceptions
	// None

// -- Contructors

	LegendreTransformCoefficientsToSpectral(const ref_counted_ptr<const LegendrePolynomials>& poly, int truncation, const vector<double>& lats);

// -- Destructor

	~LegendreTransformCoefficientsToSpectral(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods
	// None

// -- Overridden methods
	int transform(double* coeff, double* data, int latNumber) const;

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

	LegendreTransformCoefficientsToSpectral(const LegendreTransformCoefficientsToSpectral&);
	LegendreTransformCoefficientsToSpectral& operator=(const LegendreTransformCoefficientsToSpectral&);

// -- Members
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

	//friend ostream& operator<<(ostream& s,const LegendreTransformCoefficientsToSpectral& p)
	//	{ p.print(s); return s; }

};

#endif
