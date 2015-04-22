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

#include "soyuz/data/MIRField.h"
#include "soyuz/repres/Representation.h"


namespace mir {
namespace data {


MIRField::MIRField(bool hasMissing, double missingValue):
    values_(),
    hasMissing_(hasMissing),
    missingValue_(missingValue),
    representation_(0) {
}


// Warning: take ownership of values
void MIRField::values(std::vector<double>& values) {
    std::swap(values_, values);
}


MIRField::~MIRField() {
    delete representation_;
}


void MIRField::print(std::ostream& out) const {
    out << "MIRField[values=" << values_.size();
    if(hasMissing_) {
        out << ",missingValue" << missingValue_;
    }
    out << "]";
}


const repres::Representation* MIRField::representation() const {
    ASSERT(representation_);
    return representation_;
}


void MIRField::representation(repres::Representation* representation) {
    delete representation_;
    representation_ = representation;
}


}  // namespace data
}  // namespace mir

