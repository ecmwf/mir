/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef DirectFastFourierTransform_H
#define DirectFastFourierTransform_H


#ifndef FastFourierTransform_H
#include "FastFourierTransform.h"
#endif


// Forward declarations
class Grid;

class DirectFastFourierTransform : public FastFourierTransform {
  public:

// -- Exceptions
    // None

// -- Contructors

    DirectFastFourierTransform(int truncation, int lonNuber, int latsProcess, int index1, int index2);

// -- Destructor

    ~DirectFastFourierTransform(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods
    void transform(const vector<int>& offsets, vector<double>& values, const vector<double>& coeff) const;
    // None

// -- Overridden methods
    // None

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

    DirectFastFourierTransform(const DirectFastFourierTransform&);
    DirectFastFourierTransform& operator=(const DirectFastFourierTransform&);

// -- Members
    int index1_;
    int index2_;

// -- Methods
//	void getUsefulPart(double* values, const vector<int> offsets, const double* group, int jump, int multiFactor) const;
    void passThroughData(double* a, double* b, double* c, double* d, const vector<double>& trigs, int inInc, int outInc, int lot, int nLon, int factor, int productOfFactors) const;

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const DirectFastFourierTransform& p)
    //	{ p.print(s); return s; }

};

#endif
