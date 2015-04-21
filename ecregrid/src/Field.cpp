/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Field.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldDescription_H
#include "FieldDescription.h"
#endif

Field::Field():
    parameter_(0, 128, "pl"),
    units_(""),
    centre_(98),
    editionNumber_(0),
    levelType_("pl"),
    level_(500),
    date_(20000101),
    time_(1200),
    stepUnits_("h"),
    startStep_(0),
    endStep_(0),
    bitsPerValue_(0) {
}

Field::Field(int bitsPerValue, int editionNumber):
    parameter_(0, 128, "pl"),
    units_(""),
    centre_(98),
    editionNumber_(editionNumber),
    levelType_("pl"),
    level_(500),
    date_(20000101),
    time_(1200),
    stepUnits_("h"),
    startStep_(0),
    endStep_(0),
    bitsPerValue_(bitsPerValue) {
}

Field::Field(const Parameter &param) :
    parameter_(param),
    units_(""),
    centre_(98),
    editionNumber_(0),
    levelType_("pl"),
    level_(500),
    date_(20000101),
    time_(1200),
    stepUnits_("h"),
    startStep_(0),
    endStep_(0),
    bitsPerValue_(0) {
}

// Input
Field::Field(const Parameter &param, const string &units, int editionNumber, int centre, const string &levType, int level, int date, int time, const string &stepUnits, int startStep, int endStep, int bitsPerValue) :
    parameter_(param),
    units_(units),
    centre_(centre),
    editionNumber_(editionNumber),
    levelType_(levType),
    level_(level),
    date_(date),
    time_(time),
    stepUnits_(stepUnits),
    startStep_(startStep),
    endStep_(endStep),
    bitsPerValue_(bitsPerValue) {
}

// ssp to be deleted
Field::Field(const Field &f) :
    parameter_(f.parameter()),
    units_(f.units()),
    centre_(f.centre()),
    editionNumber_(f.editionNumber()),
    levelType_(f.levelType()),
    level_(f.level()),
    date_(f.date()),
    time_(f.time()),
    stepUnits_(f.stepUnits()),
    startStep_(f.startStep()),
    endStep_(f.endStep()),
    bitsPerValue_(f.bitsPerValue()) {
}

Field::Field(const Field &f, int bitsPerValue, int editionNumber) :
    parameter_(f.parameter()),
    units_(f.units()),
    centre_(f.centre()),
    editionNumber_(editionNumber),
    levelType_(f.levelType()),
    level_(f.level()),
    date_(f.date()),
    time_(f.time()),
    stepUnits_(f.stepUnits()),
    startStep_(f.startStep()),
    endStep_(f.endStep()),
    bitsPerValue_(bitsPerValue) {
    if (!bitsPerValue_)
        bitsPerValue_ = f.bitsPerValue();
    if (!editionNumber_)
        editionNumber_ = f.editionNumber();

    // it could be set by grib_api
    //  ASSERT(editionNumber_);
}
// setting u,v in SpectralToGridTransformer
Field::Field(const Parameter &param, const Field &f, int bitsPerValue, int editionNumber) :
    parameter_(param),
    units_(f.units()),
    centre_(f.centre()),
    editionNumber_(editionNumber),
    levelType_(f.levelType()),
    level_(f.level()),
    date_(f.date()),
    time_(f.time()),
    stepUnits_(f.stepUnits()),
    startStep_(f.startStep()),
    endStep_(f.endStep()),
    bitsPerValue_(bitsPerValue) {
    if (!bitsPerValue_)
        bitsPerValue_ = f.bitsPerValue();
    if (!editionNumber_)
        editionNumber_ = f.editionNumber();

    // it could be set by grib_api
    //  ASSERT(editionNumber_);
}

Field::Field(const Parameter &param, const Field &f) :
    parameter_(param),
    units_(f.units()),
    centre_(f.centre()),
    editionNumber_(f.editionNumber()),
    levelType_(f.levelType()),
    level_(f.level()),
    date_(f.date()),
    time_(f.time()),
    stepUnits_(f.stepUnits()),
    startStep_(f.startStep()),
    endStep_(f.endStep()),
    bitsPerValue_(f.bitsPerValue()) {
}


Field::~Field() {
}

bool Field::operator==(const Field &input) const {
    return isSameLevel(input.level_) && isSameLevelType(input.levelType_);
}

bool Field::isSameLevelType( const string &lt) const {
    if (lt == "sfc" && levelType_ == "sfc")
        return true;

    if (levelType_ != "sfc")
        if (lt == levelType_ )
            return true;
        else
            return false;
    else
        return true;
}

void Field::setOutputAreaAndBasics(FieldDescription &out) const {
    out.parameter(parameter_);
    out.centre(centre_);
    out.date(date_);
    out.time(time_);
}

bool Field::isSameLevel( int lev) const {
    if (lev == 0 && level_ == 0)
        return true;

    if (level_ > 0)
        if (lev == level_)
            return true;
        else
            return false;
    else
        return true;
}


void Field::print(ostream &out) const {
    out <<  parameter_ << ", levelType=[" << levelType_ <<  "], level=[" << level_ << "], ";
    out << "editionNumber=[" << editionNumber_ << "], bitsPerValue=[" << bitsPerValue_ <<  "], centre=[" << centre_ << "]";
}
