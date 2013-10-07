/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef FieldFeatures_H
#define FieldFeatures_H

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#ifndef Area_H
#include "Area.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef Parameter_H
#include "Parameter.h"
#endif

// 

struct FieldFeatures {

public:

// -- Contructors
	FieldFeatures();

// -- Destructor

	~FieldFeatures();

// -- Operators
	FieldFeatures& operator=(const FieldFeatures& other);

	int       scanningMode_;
	bool      isReduced_;
	bool      decide_;

	int       parameterId_;
	int       table_;

	string    levelType_;
	int       level_;

	string    units_;
	int       centre_;
	int       editionNumber_;

	int       date_;
	int       time_;

	string    stepUnits_;
	int       startStep_;
	int       endStep_;

	int       bitsPerValue_;

	int       gaussianNumber_;
	int       truncation_;
	double    we_;
	double    ns_;

	long      nptsNS_;
	long      nptsWE_;

	bool      globalWestEast_;
	bool      isSetglobalWestEast_;
	bool      global_;

	Point     southPole_;
	Area      area_;

	Parameter parameter_;

    long* reducedGridDefinition_;
	size_t      reducedGridLatNumber_;

	bool      frame_;
	int       frameNumber_;
	bool      bitmap_;
	string    bitmapFile_;

	string    listOfPointsFileType_;
	string    listOfPointsFile_;

	bool      auresol_;
	int       fftMax_;

	int       numberOfNearestPoints_;

	string    grid2gridTransformationType_;

	string    interpolationMethod_;
	string    lsmMethod_;
	string    legendrePolynomialsMethod_;
	
	string    extrapolateOnPole_;

	bool      vdConversion_;

	bool      pseudoGaussian_;
	bool      cellCentered_;
	bool      shifted_;

	int       projectionPredefines_;

	string    fileType_;

	double    missingValue_;

// -- Methods
	double  north()           const { return area_.north(); }
    double  south()           const { return area_.south(); }
    double  west()            const { return area_.west(); }
    double  east()            const { return area_.east(); }

    int     date()            const { return date_ ; }
    int     time()            const { return time_ ; }

    double  poleLat()         const { return southPole_.latitude(); }
    double  poleLon()         const { return southPole_.longitude(); }


	const Parameter& parameter()   const { return parameter_; }
    int     parameterNumber() const { return parameter_.number() ; }
    int     parameterTable()  const { return parameter_.table() ; }

	void copyBasics(const FieldFeatures& f);



protected:
// -- Methods
	virtual void print(ostream&) const;

// -- Friends

	friend ostream& operator<<(ostream& s,const FieldFeatures& p)
		{ p.print(s); return s; }
};

#endif
