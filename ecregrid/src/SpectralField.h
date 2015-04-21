/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef SpectralField_H
#define SpectralField_H


// Headers

#ifndef Field_H
#include "Field.h"
#endif

#include <complex>
typedef complex<double> comp;

// Forward declarations
class Parameter;
class FieldDescription;

//

class SpectralField : public Field {
  public:

// -- Contructors

    SpectralField( int truncation );

    SpectralField(int truncation, const Parameter& param, const string& units, int editionNumber, int centre, const string& levType, int level,int date, int time, const string& stepUnits, int startStep, int endStep, int bitsPerValue, const vector<double>& values);

    SpectralField(int truncation, const Field& f, const vector<double>& values);
    SpectralField(int truncation, const Parameter& param, const Field& f, const vector<double>& values);

// -- Destructor

    ~SpectralField(); // Change to virtual if base class


// -- Operators
    bool operator==(const SpectralField& other) const;

// -- Methods
    bool           isRotated()                        const {
        return false;
    }
    unsigned long  calculatedLength()                 const {
        return (truncation_ + 1) * (truncation_ + 2);
    }
    string         nameOfField()                      const {
        return "sh";
    }
    int            truncation()                       const {
        return truncation_;
    }

    void           getDataAsComplex(vector<comp>& comp) const;
    size_t dataLengthComplex() const;
    int matchGaussian() const;
    double matchLatLon() const;

// -- Overridden methods
    void              resetOutput(FieldDescription* out) const;
    FieldDescription* makeOutput(const FieldDescription& out)  const;
    const vector<double>&  data()                             const {
        return data_;
    }
    size_t dataLength()                       const {
        return data_.size();
    }
    void           dump2file(const string& name)      const;
    void  outputArea(double* a)      const;
    void  setOutputAreaAndBasics(FieldDescription& out) const;

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members
    // None

// -- Methods

    void print(ostream&) const; // Change to virtual if base class

  private:

// No copy allowed

    SpectralField(const SpectralField&);
    SpectralField& operator=(const SpectralField&);

// -- Members
    int      truncation_;

    //const vector<double>& data_;
    const vector<double> data_;

// -- Methods
    // None

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend ostream& operator<<(ostream& s,const SpectralField& p) {
        p.print(s);
        return s;
    }

};

#endif
