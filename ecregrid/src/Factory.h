/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Factory_H
#define Factory_H


// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#include "LegendrePolynomialsCollection.h"

// Forward declarations
class Input;
class Output;
class Lsm;
class Field;
class Transformer;
class LegendrePolynomials;
class Grid;
class GridField;
class GribApiOutput;
class DerivedSubgridParameters;
class Interpolator;
class Extraction;
class FieldDescription;
//

class Factory {
  public:

// -- Contructors

    Factory();

// -- Destructor

    ~Factory(); // Change to virtual if base class

// -- Methods
    DerivedSubgridParameters* selectDerivedSubgridParameter(const string& param) const;

    Transformer* getTransformer(const Field& in,const Field& out, const FieldDescription& fd) const;

    Input*       getInput(const string& fileName, const string& kind = "grib")   const;

    GribApiOutput* getGribApiOutput(const string& n)  const;
    GribApiOutput* getGribApiOutput(const string& fileName,const string& n)  const;
    Output*        getOutput(const string& fileName, const string& kind, const string& name)  const;
    Output*        getOutputBinTxt(const string& fileName, const string& kind)  const;

    Lsm*         getLsm(const string& fileName, const string& lsmMethod, const string& lsmFileType, bool user, const string& userPath) const;
    ref_counted_ptr<const LegendrePolynomials> polynomialsMethod(const string& method, int truncation, const Grid& grid) const;
    Extraction*   multiExtraction(const GridField& out, double missingValue)  const;
    Extraction*   multiExtractionWithoutSubArea(const GridField& out, double missingValue)  const;

    Interpolator* interpolationMethod(const string& method, int pointsForInterpolation, const GridField& input, const Grid& gridout, bool lsm, const string& lsmMethod, const string& extrapolate, double missingValue, bool bitmap) const;

  private:

// No copy allowed

    Factory(const Factory&);
    Factory& operator=(const Factory&);

    static LegendrePolynomialsCollection memoryMapCache_;
    static LegendrePolynomialsCollection sharedMemoryCache_;

};

#endif
