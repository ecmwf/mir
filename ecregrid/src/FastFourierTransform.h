/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef FastFourierTransform_H
#define FastFourierTransform_H

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#include <vector>

// Forward declarations
class Grid;

//
static const double sin36 = 0.587785252292473;
static const double sin60 = 0.866025403784437;
static const double sin72 = 0.951056516295154;
static const double qrt5  = 0.559016994374947;

class FastFourierTransform {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    FastFourierTransform(int truncation, int lonNuber, int latsProcess, int maxBloxSize = 64);

    // -- Destructor

    virtual ~FastFourierTransform(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    //	virtual void transform(double* values, const mapLats& mapped, const Grid& grid) const = 0;
    //	virtual void transform(const vector<int>& offsets, double* values, const double* legendreCoeff) const = 0;

    void setSinesAndCosinesFromZeroToPi(vector<double> &trigs, int n) const;
    void factorize(vector<int> &f, int n) const;
    int  adjustNumberOfPointsAlongLatitude(int *factor) const;

    // -- Overridden methods


  protected:

    // -- Members
    int truncation_;
    int longitudeNumber_;
    int latsProcess_;
    int maxBloxSize_;

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

    FastFourierTransform(const FastFourierTransform &);
    FastFourierTransform &operator=(const FastFourierTransform &);

    // -- Members

    // -- Methods

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const FastFourierTransform& p)
    //	{ p.print(s); return s; }

};

#endif
