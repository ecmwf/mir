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

#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/data/MIRFieldStats.h"


namespace mir {
namespace data {


MIRField::MIRField(const param::MIRParametrisation &param, bool hasMissing, double missingValue):
    parent_(0),
    values_(),
    hasMissing_(hasMissing),
    missingValue_(missingValue),
    representation_(repres::RepresentationFactory::build(param)) {

    if (representation_) {
        representation_->attach();
    }
}


MIRField::MIRField(const repres::Representation *repres, bool hasMissing, double missingValue):
    parent_(0),
    values_(),
    hasMissing_(hasMissing),
    missingValue_(missingValue),
    representation_(repres) {

    if (representation_) {
        representation_->attach();
    }
}


MIRField::MIRField(MIRField *parent):
    parent_(parent),
    values_(),
    hasMissing_(parent->hasMissing_),
    missingValue_(parent->missingValue_),
    representation_(parent->representation_) {

    if (representation_) {
        representation_->attach();
    }
}

void MIRField::copyOnWrite() {
    if (parent_) {
        MIRField *top = this;
        while (top->parent_) {
            top = top->parent_;
        }
        values_ = top->values_;
        paramId_ = top->paramId_;
        parent_ = 0;
        // std::cout << "MIRField::copyOnWrite" << std::endl;
    }
}

// Warning: take ownership of values
void MIRField::update(std::vector<double> &values, size_t which) {
    copyOnWrite();
    if (values_.size() <= which) {
        values_.resize(which + 1);
    }
    std::swap(values_[which], values);
}

size_t MIRField::dimensions() const {
    if (parent_) {
        return parent_->dimensions();
    }
    return values_.size();
}


void MIRField::dimensions(size_t size)  {
    copyOnWrite();
    return values_.resize(size);
}


MIRField::~MIRField() {
    if (representation_) {
        representation_->detach();
    }
}


void MIRField::print(std::ostream &out) const {

    out << "MIRField[dimensions=" << values_.size();
    if (hasMissing_) {
        out << ",missingValue=" << missingValue_;
    }

    if (representation_) {
        out << ",representation=" << *representation_;
    }

    if (paramId_.size()) {
        out << ",params=";
        char sep = '(';
        for (size_t i = 0; i < paramId_.size(); i++) {
            out << sep << paramId_[i];
            sep = ',';
        }
        out << ')';
    }

    if (parent_) {
        out << ",parent=";
        parent_->print(out);
        out << "]";
        return;
    }

    out << "]";
}


const repres::Representation *MIRField::representation() const {
    ASSERT(representation_);
    return representation_;
}

void MIRField::validate() const {

    if (representation_) {
        for (size_t i = 0; i < values_.size(); i++) {
            representation_->validate(values(i));
        }
    }
}

MIRFieldStats MIRField::statistics(size_t i) const {

    if (parent_) {
        return parent_->statistics(i);
    }

    if (hasMissing_) {
        const std::vector<double> &vals = values(i);
        std::vector<double> tmp;
        tmp.reserve(vals.size());
        size_t missing = 0;

        for (size_t j = 0; j < vals.size(); j++) {
            if (vals[j] != missingValue_) {
                tmp.push_back(vals[j]);
            } else {
                missing++;
            }
        }
        return MIRFieldStats(tmp, missing);
    }
    return MIRFieldStats(values(i), 0);
}

void MIRField::representation(const repres::Representation *representation) {
    if (representation) {
        representation->attach();
    }
    if (representation_) {
        representation_->detach();
    }
    representation_ = representation;
}

const std::vector<double> &MIRField::values(size_t which) const {

    if (parent_) {
        return parent_->values(which);
    }

    ASSERT(which < values_.size());
    return values_[which];
}

std::vector<double> &MIRField::direct(size_t which)  {

    copyOnWrite();

    ASSERT(which < values_.size());
    return values_[which];
}

void MIRField::paramId(size_t which, size_t param) {
    copyOnWrite();
    while (paramId_.size() <= which) {
        paramId_.push_back(0);
    }
    paramId_[which] = param;
}

size_t MIRField::paramId(size_t which) const {
    if (parent_) {
        return parent_->paramId(which);
    }

    if (paramId_.size() <= which) {
        return 0;
    }

    return paramId_[which];
}

bool MIRField::hasMissing() const {
    return hasMissing_;
}

double MIRField::missingValue() const {
    return missingValue_;
}


void MIRField::hasMissing(bool on) {
    hasMissing_ = on;
}

void MIRField::missingValue(double value)  {
    missingValue_ = value;
}

}  // namespace data
}  // namespace mir

