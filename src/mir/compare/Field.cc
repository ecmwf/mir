/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <iostream>
#include <cmath>

#include "mir/compare/Field.h"
#include "mir/compare/FieldSet.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Colour.h"

#include "mir/compare/BufrField.h"
#include "mir/compare/GribField.h"

namespace mir {
namespace compare {


void Field::addOptions(std::vector<eckit::option::Option*>& options) {
    GribField::addOptions(options);
    BufrField::addOptions(options);
}


void Field::setOptions(const eckit::option::CmdArgs &args) {
    GribField::setOptions(args);
    BufrField::setOptions(args);
}


Field::Field(FieldBase* field):
    field_(field)
{
    if (field_) {
        field_->attach();
    }
}


Field::Field(const Field& other):
    field_(other.field_)
{
    if (field_) {
        field_->attach();
    }
}

Field::~Field() {
    if (field_) {
        field_->detach();
    }
}


Field& Field::operator=(const Field& other)
{
    if (field_ != other.field_) {
        if (field_) {
            field_->attach();
        }
        field_ = other.field_;
        if (field_) {
            field_->attach();
        }
    }

    return *this;
}

void Field::print(std::ostream& out) const {
    if (field_) {
        out << *field_;
    }
    else {
        out << "(null)";
    }
}


std::vector<Field> Field::bestMatches(const FieldSet & fields) const {
    std::vector<Field> matches;

    for (auto k = fields.begin(); k != fields.end(); ++k) {
        const auto& other = *k;

        if (match(other)) {
            matches.push_back(other);
        }

    }

    return matches;

}


bool Field::same(const Field& other) const {
    NOTIMP;
}

bool Field::match(const Field& other) const {
    NOTIMP;
}


void Field::whiteListEntries(std::ostream&) const {
    NOTIMP;
}

size_t Field::differences(const Field& other) const {
    NOTIMP;
}

void Field::printDifference(std::ostream&, const Field& other) const {
    NOTIMP;
}

Field::operator bool() const {
    return field_ != 0;
}

bool Field::operator<(const Field& other) const {
    NOTIMP;
}

bool Field::wrapped() const {
    NOTIMP;
}

void Field::compareAreas(std::ostream&, const Field& other) const {
    NOTIMP;
}

off_t Field::offset() const {
    ASSERT(field_);
    return field_->offset();
}

size_t Field::length() const {
    ASSERT(field_);
    return field_->length();
}

const std::string& Field::path() const {
    ASSERT(field_);
    return field_->path();
}


//----------------------------------------------------------------------------------------------------------------------


FieldBase::FieldBase(const std::string& path, off_t offset, size_t length):
    info_(path, offset, length) {

}

off_t FieldBase::offset() const {
    return info_.offset();
}

size_t FieldBase::length() const {
    return info_.length();
}

const std::string& FieldBase::path() const {
    return info_.path();
}


//----------------------------------------------------------------------------------------------------------------------
}  // namespace compare

}  // namespace mir
