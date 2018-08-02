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
    ASSERT(field_);
    field_->attach();
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
