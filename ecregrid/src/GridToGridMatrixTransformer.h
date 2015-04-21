/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef GridToGridMatrixTransformer_H
#define GridToGridMatrixTransformer_H

#ifndef   Transformer_H
#include "Transformer.h"
#endif


// Forward declarations
class Field;
class GridField;
class Grid;
class Extraction;

class GridToGridMatrixTransformer : public Transformer {
  public:

// -- Contructors

    GridToGridMatrixTransformer(const string& intMethod = "default", const string& lsmMethod = "10min", int npts = 16, const string& type = "interpolation", const string& extrapolate = "default", double missingValue = MISSING_VALUE);

// -- Destructor

    ~GridToGridMatrixTransformer(); // Change to virtual if base class

// -- Methods
    //bool   transform(const GridField& in, const Grid& outGrid, const Extraction* extraction, vector<double>& values) const;

// -- Overridden methods
    Field* transform(const Field& input, const Field& output ) const;
    Wind*  transformVector(const Field& inU, const Field& inV, const Field& req) const;

  protected:

// -- Members
    // None

// -- Methods

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    GridToGridMatrixTransformer(const GridToGridMatrixTransformer&);
    GridToGridMatrixTransformer& operator=(const GridToGridMatrixTransformer&);

// -- Members
    string interpolationMethod_;
    string lsmMethod_;
    int    pointsForInterpolation_;
    string type_;
    string extrapolate_;

    double missingValue_;

// -- Methods
    // None

// -- Class members
    // None

// -- Class methods
    // None


};

#endif
