/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef InverseFastFourierTransform_H
#define InverseFastFourierTransform_H

#ifndef FastFourierTransform_H
#include "FastFourierTransform.h"
#endif


// Forward declarations
class Grid;

class InverseFastFourierTransform : public FastFourierTransform {
public:

// -- Exceptions
	// None

// -- Contructors

	InverseFastFourierTransform(int truncation, int lonNumber, int latsNumProc, double west, double east, bool wind);
	InverseFastFourierTransform(int truncation, int lonNumber, int latsNumProc, int maxBloxSize, double west, double east);

// -- Destructor

	~InverseFastFourierTransform(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods

// -- Overridden methods
	long transform(vector<double>& values,  const mapLats& mapped, const Grid& grid) const;

// -- Class members
	// None

// -- Class methods
	// None

protected:

// -- Members
	// None

// -- Methods
	
//	 void print(ostream&) const; // Change to virtual if base class	

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	InverseFastFourierTransform(const InverseFastFourierTransform&);
	InverseFastFourierTransform& operator=(const InverseFastFourierTransform&);

// -- Members

// -- Methods
	long getUsefulPart(vector<double>& values, const vector<int>& offsets, const vector<double>& group, int jump, int multiFactor) const;
	long getUsefulPartWind(vector<double>& values, const vector<int>& offsets, const vector<double>& group, int jump, int multiFactor, const vector<double>& lats) const;

// -- Overridden methods
	void passThroughData(double* a, double* b, double* c, double* d, const vector<double>& trigs, int inInc, int outInc, int lot, int nLon, int factor, int productOfFactors) const;

// -- Class members
	double west_;
	double east_;

	bool wind_;

// -- Class methods
	// None

// -- Friends

//	friend ostream& operator<<(ostream& s,const InverseFastFourierTransform& p)
//		{ p.print(s); return s; }
};

#endif
