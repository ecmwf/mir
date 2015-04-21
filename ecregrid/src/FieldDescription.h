/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef FieldDescription_H
#define FieldDescription_H

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#ifndef FieldIdentity_H
#include "FieldIdentity.h"
#endif

#ifndef FieldFeatures_H
#include "FieldFeatures.h"
#endif

// Forward declarations
class Grid;
class Point;
class Area;
class Field;
class Parameter;
//

struct FieldDescription {

    // -- Contructors
    FieldDescription();
    FieldDescription(const string &composedName);

    // -- Destructor

    virtual ~FieldDescription(); // Change to virtual if base class

    // -- Methods

    double  north()         const {
        return ft_.area_.north();
    }
    double  south()         const {
        return ft_.area_.south();
    }
    double  west()          const {
        return ft_.area_.west();
    }
    double  east()          const {
        return ft_.area_.east();
    }
    int  parameterNumber()  const {
        return ft_.parameterNumber();
    }
    int  parameterTable()   const {
        return ft_.parameterTable();
    }

    int  date()             const {
        return ft_.date();
    }
    int  time()             const {
        return ft_.time();
    }

    int scanningMode()      const {
        return ft_.scanningMode_;
    }
    int gaussianNumber()    const {
        return ft_.gaussianNumber_;
    }
    int truncation()        const {
        return ft_.truncation_;
    }

    double westEastInc()    const {
        return ft_.we_;
    }
    double northSouthInc()  const {
        return ft_.ns_;
    }

    long northSouthNumberOfPoints() const {
        return ft_.nptsNS_;
    }
    long westEastNumberOfPoints()   const {
        return ft_.nptsWE_;
    }

    bool isGlobalWestEast() const {
        return ft_.globalWestEast_;
    }

    double southPoleLat()   const {
        return ft_.southPole_.latitude();
    }
    double southPoleLon()   const {
        return ft_.southPole_.longitude();
    }

    bool   isRotated()      const {
        return ft_.southPole_.latitude() != 0 || ft_.southPole_.longitude() != 0;
    }

    bool decide()           const {
        return ft_.decide_;
    }

    bool   bitmap()         const {
        return ft_.bitmap_;
    }
    string bitmapFile()     const {
        return ft_.bitmapFile_;
    }
    bool   frame()          const {
        return ft_.frame_;
    }
    int    frameNumber()    const {
        return ft_.frameNumber_;
    }

    bool   cellCentered()   const {
        return ft_.cellCentered_;
    }
    bool   shifted()        const {
        return ft_.shifted_;
    }

    bool   vdConversion()   const {
        return ft_.vdConversion_;
    }

    string fileType()       const {
        return ft_.fileType_;
    }
    double missingValue()   const {
        return ft_.missingValue_;
    }

    Field *defineField() const;


    // -- API functions
    void missingValue(double value);
    void grid2gridTransformationType(const string &s);
    void parameter(const Parameter &param);
    void gridType(const string &type);

    void area(double north, double west, double south, double east);
    void increments(double we, double ns);
    void gaussianNumber(int number);
    void truncation(int number);

    void numberOfPoints(long ns, long we);

    void globalWestEast(const string &str);

    void southPole(double lat, double lon);
    void scanningMode(int scanningMode);

    void isAvailable();
    void ifAvailableCopyBasics(const FieldDescription &other);

    void isReduced(const bool isReduced);
    void levelType(const string &level);
    void level(int level);

    void units(const string &units);
    void centre(int centre);
    void editionNumber(int editionNumber);

    void stepUnits(const string &stepUnits);
    void startStep(int startStep);
    void endStep(int endStep);

    void date(int date);
    void time(int time);

    void reducedGridDefinition(long *rgridDef, size_t size);

    void frameNumber(int number);
    void bitmapFile(const string &file);

    void listOfPointsFileType(const string &fileType);
    void listOfPointsFile(const string &file);

    void interpolationMethod(const string &method);
    void lsmMethod(const string &method);
    void legendrePolynomialsMethod(const string &method);

    void auresol(const string &method);
    void fftMax(int b);

    void numberOfNearestPoints(int b);

    void bitsPerValue(int b);
    void vdConversion(const bool conv);

    void cellCentered(bool a, bool b);
    void gridSpec(const string &spec);

    void shifted(bool b);

    void extrapolateOnPole(const string &method);
    void projectionPredefines(int number);

    void fileType(const string &type);

    void global(bool b);
    void parameterId(int b);
    void table(int b);
    void pseudoGaussian(bool b);

    virtual void print(ostream &) const; // Change to virtual if base class

    FieldDescription &operator=(const FieldDescription &);

    // -- Class members
    FieldIdentity  id_;
    FieldFeatures  ft_;

    // -- Friends

    friend ostream &operator<<(ostream &s, const FieldDescription &p) {
        p.print(s);
        return s;
    }
};

#endif
