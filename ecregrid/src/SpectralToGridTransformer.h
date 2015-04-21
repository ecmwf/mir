/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef SpectralToGridTransformer_H
#define SpectralToGridTransformer_H

#ifndef   Transformer_H
#include "Transformer.h"
#endif

#include <complex>
typedef complex<double> comp;

// Forward declarations
class Field;
class Grid;
class wind;


class SpectralToGridTransformer : public Transformer {
  public:

// -- Exceptions
    // None

// -- Contructors

    SpectralToGridTransformer(const string& legendrePolynomialsMethod, int fftMax, bool auresol, bool conversion);

// -- Destructor

    virtual ~SpectralToGridTransformer(); // Change to virtual if base class


// -- Overridden methods
    virtual Field* transform(const Field& in, const Field& out) const;
    virtual Wind* transformVector(const Field& inU, const Field& inV, const Field& req) const;

  protected:

// -- Members
    bool   vdConversion_;
    bool   auresol_;
// -- Methods
    void transform(int truncation, bool  windParameter, const vector<comp>& dataComplex, const Grid& output, const Grid& outputGlobal, vector<double>& values) const;

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

    SpectralToGridTransformer(const SpectralToGridTransformer&);
    SpectralToGridTransformer& operator=(const SpectralToGridTransformer&);

// -- Members
    string legendrePolynomialsMethod_;
    int    fftMaxBlockSize_;

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend ostream& operator<<(ostream& s,const SpectralToGridTransformer& p) {
        p.print(s);
        return s;
    }

};

#endif
