/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef SpectralToRotatedGridTransformer_H
#define SpectralToRotatedGridTransformer_H

#ifndef   SpectralToGridTransformer_H
#include "SpectralToGridTransformer.h"
#endif

class SpectralToRotatedGridTransformer : public SpectralToGridTransformer {
  public:

// -- Exceptions
    // None

// -- Contructors

    SpectralToRotatedGridTransformer(const string& coeffMethod, int fftMax, bool auresol, bool conversion);

// -- Destructor

    ~SpectralToRotatedGridTransformer(); // Change to virtual if base class

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

    // void print(ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    SpectralToRotatedGridTransformer(const SpectralToRotatedGridTransformer&);
    SpectralToRotatedGridTransformer& operator=(const SpectralToRotatedGridTransformer&);

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

    //friend ostream& operator<<(ostream& s,const SpectralToRotatedGridTransformer& p)
    //	{ p.print(s); return s; }

};

#endif
