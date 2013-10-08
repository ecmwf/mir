/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "FieldFeatures.h"

FieldFeatures::FieldFeatures() :
	scanningMode_(1),
    isReduced_(false),
    decide_(false),
    parameterId_(172),
    table_(128),
    levelType_("sfc"),
    level_(0),
    units_("default"),
    centre_(98),
    editionNumber_(0),
    date_(20000101),
    time_(1200),
    stepUnits_("h"),
    startStep_(0),
    endStep_(0),
    bitsPerValue_(16),
    gaussianNumber_(0),
    truncation_(0),
    we_(0),
    ns_(0),
    nptsNS_(0),
    nptsWE_(0),
    globalWestEast_(true),
    isSetglobalWestEast_(false),
    global_(true),
    southPole_(0,0),
    area_(0,0,0,0),
    parameter_(),
    reducedGridDefinition_(0),
    reducedGridLatNumber_(0),
    frame_(false), 
    frameNumber_(0), 
    bitmap_(false), 
    bitmapFile_("none"),
    listOfPointsFileType_("ascii"), 
    listOfPointsFile_("none"), 
    auresol_(false),
    fftMax_(64),
    numberOfNearestPoints_(4), 
    grid2gridTransformationType_("interpolation"),
    interpolationMethod_("default"),
    lsmMethod_("default"),
    legendrePolynomialsMethod_("default"),
    extrapolateOnPole_("default"),
    vdConversion_(true),
    pseudoGaussian_(false),
    cellCentered_(false),
    shifted_(false),
    projectionPredefines_(100),
    fileType_("binary"),
    missingValue_(MISSING_VALUE)
{
}

FieldFeatures::~FieldFeatures()
{
//	delete [] reducedGridDefinition_;
}

FieldFeatures& FieldFeatures::operator=(const FieldFeatures& f)
{
	if (this == &f)
		return *this;
	scanningMode_    = f.scanningMode_;
	isReduced_       = f.isReduced_;

	date_            = f.date_;
	time_            = f.time_;

	units_           = f.units_;
	centre_          = f.centre_;
	editionNumber_   = f.editionNumber_;
	bitsPerValue_    = f.bitsPerValue_;

	stepUnits_       = f.stepUnits_;
	startStep_       = f.startStep_;
	endStep_         = f.endStep_;

	levelType_       = f.levelType_;
	level_           = f.level_;

	decide_          = f.decide_;
	area_            = f.area_;
	gaussianNumber_  = f.gaussianNumber_;
	truncation_      = f.truncation_;
	we_              = f.we_;
	ns_              = f.ns_;

	nptsNS_          = f.nptsNS_;
	nptsWE_          = f.nptsWE_;

	globalWestEast_  = f.globalWestEast_;
	isSetglobalWestEast_ = f.isSetglobalWestEast_;

	southPole_       = f.southPole_;

	reducedGridDefinition_ = f.reducedGridDefinition_;
	reducedGridLatNumber_  = f.reducedGridLatNumber_;

	frame_           = f.frame_;
	frameNumber_     = f.frameNumber_;
	bitmap_          = f.bitmap_;
	bitmapFile_      = f.bitmapFile_;

	listOfPointsFileType_  = f.listOfPointsFileType_;
	listOfPointsFile_      = f.listOfPointsFile_;

	interpolationMethod_       = f.interpolationMethod_;
	lsmMethod_                 = f.lsmMethod_;
	legendrePolynomialsMethod_ = f.legendrePolynomialsMethod_;

	auresol_               = f.auresol_;
	fftMax_                = f.fftMax_;

	numberOfNearestPoints_ = f.numberOfNearestPoints_;

	vdConversion_          = f.vdConversion_;

	grid2gridTransformationType_  = f.grid2gridTransformationType_;


	cellCentered_ = f.cellCentered_;
	shifted_      = f.shifted_;

	extrapolateOnPole_      = f.extrapolateOnPole_;

	projectionPredefines_  = f.projectionPredefines_;

	fileType_  = f.fileType_;
	missingValue_  = f.missingValue_;

	global_          = f.global_;
	parameterId_     = f.parameterId_;
	table_           = f.table_;
	pseudoGaussian_  = f.pseudoGaussian_;

	return *this;
}

void FieldFeatures::copyBasics(const FieldFeatures& f)
{
	date_            = f.date_;
	time_            = f.time_;

	units_           = f.units_;
	centre_          = f.centre_;
	editionNumber_   = f.editionNumber_;

	stepUnits_       = f.stepUnits_;
	startStep_       = f.startStep_;
	endStep_         = f.endStep_;

	levelType_       = f.levelType_;
	level_           = f.level_;
}

void FieldFeatures::print(ostream&) const
{
	cout << "FIELD FEATURES { Area=[" << area_ << "], Gaussian Number=[" << gaussianNumber_ << "], Truncation=[" << truncation_ << "], we=[" << we_ << "], ns=[" << ns_ << "], southPole=[" << southPole_ << "], isReduced=[" << isReduced_ << "] }";
}
