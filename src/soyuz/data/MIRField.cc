// File MIRField.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/data/MIRField.h"
#include "soyuz/repres/Representation.h"

#include "eckit/exception/Exceptions.h"

#include <iostream>

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

const Representation* MIRField::representation() const {
    ASSERT(representation_);
    return representation_;
}

void MIRField::representation(Representation* representation) {
    delete representation_;
    representation_ = representation;
}
