/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "mir/data/Field.h"
#include "mir/data/MIRField.h"
#include "mir/data/MIRFieldStats.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace data {


MIRField::MIRField(const param::MIRParametrisation& param, bool hasMissing, double missingValue) :
    field_(new Field(param, hasMissing, missingValue)) {
    field_->attach();
}


MIRField::MIRField(const repres::Representation* repres, bool hasMissing, double missingValue) :
    field_(new Field(repres, hasMissing, missingValue)) {
    field_->attach();
}


MIRField::MIRField(const MIRField& other) :
    field_(other.field_) {
    field_->attach();
}


void MIRField::copyOnWrite() {
    if (field_->count() > 1) {
        // eckit::Log::info() << "XXXX copyOnWrite " << *field_ << std::endl;
        Field *f = field_->clone();
        field_->detach();
        field_ = f;
        field_->attach();
    }
}


// Warning: take ownership of values
void MIRField::update(std::vector<double> &values, size_t which) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    // eckit::Log::info() << "MIRField::update " << *field_ << std::endl;

    copyOnWrite();
    field_->update(values, which);
}


size_t MIRField::dimensions() const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    return field_->dimensions();
}


void MIRField::dimensions(size_t size)  {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    copyOnWrite();
    field_->dimensions(size);
}


void MIRField::select(size_t which)  {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);
    // TODO: Check the if we can select() wothout copying everything first
    copyOnWrite();
    field_->select(which);
}


MIRField::~MIRField() {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    field_->detach();
}


void MIRField::print(std::ostream &out) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    out << *field_;
}


const repres::Representation *MIRField::representation() const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    return field_->representation();
}


void MIRField::validate() const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    field_->validate();
}


MIRFieldStats MIRField::statistics(size_t i) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    return field_->statistics(i);
}


void MIRField::representation(const repres::Representation *representation) {
    // eckit::Log::info() << "MIRField::representation " << *field_ << " => " << *representation << std::endl;
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    copyOnWrite();
    field_->representation(representation);
}


const std::vector<double> &MIRField::values(size_t which) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    return field_->values(which);
}


std::vector<double> &MIRField::direct(size_t which)  {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    // eckit::Log::info() << "MIRField::direct " << *field_ << std::endl;
    copyOnWrite();
    return field_->direct(which);
}


void MIRField::metadata(size_t which, const std::map<std::string, long>& md) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    // eckit::Log::info() << "MIRField::paramId " << *field_ << std::endl;

    copyOnWrite();
    field_->metadata(which, md);
}

void MIRField::metadata(size_t which, const std::string& name, long value) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    // eckit::Log::info() << "MIRField::paramId " << *field_ << std::endl;

    copyOnWrite();
    field_->metadata(which, name, value);
}

const std::map<std::string, long>& MIRField::metadata(size_t which) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    return field_->metadata(which);
}


bool MIRField::hasMissing() const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    return field_->hasMissing();
}


double MIRField::missingValue() const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    return field_->missingValue();
}


void MIRField::hasMissing(bool on) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    if (on != hasMissing()) {
        copyOnWrite();
        field_->hasMissing(on);
    }
}


void MIRField::missingValue(double value)  {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    if (value != missingValue()) {
        copyOnWrite();
        field_->missingValue(value);
    }
}


}  // namespace data
}  // namespace mir

