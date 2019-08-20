/*
 * (C) Copyright 1996- ECMWF.
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

#include "mir/api/MIREstimation.h"

namespace mir {
namespace api {


MIREstimation::MIREstimation():
    totalNumberOfFields_(0),
    totalNumberOfGridPoints_(0),
    totalNumberOfBits_(0)
{
}


MIREstimation::~MIREstimation() = default;


void MIREstimation::startField() {
    numberOfGridPoints_ = 0;
    accuracy_ = 0;
    bitsPerValue_ = 0;
    edition_ = 0;
    truncation_ = 0;
    packing_.clear();
}

void MIREstimation::endField() {
    totalNumberOfFields_ += 1;
    totalNumberOfGridPoints_ += numberOfGridPoints_;

    totalNumberOfBits_ += accuracy_ * numberOfGridPoints_;
}

void MIREstimation::numberOfGridPoints(size_t cnt) {
    numberOfGridPoints_ = cnt;
    truncation_ = 0;
}


void MIREstimation::accuracy(size_t bits) {
    accuracy_ = bits;
}

void MIREstimation::edition(size_t edition) {
    edition_ = edition;
}


void MIREstimation::truncation(size_t truncation) {
    numberOfGridPoints_ = 0;
    truncation_ = truncation;
}

void MIREstimation::packing(const std::string& packing) {
    packing_ = packing;
}

void MIREstimation::print(std::ostream &out) const {
    out << "MIREstimation["
        << "totalNumberOfFields=" << totalNumberOfFields_
        << ",totalNumberOfGridPoints=" << totalNumberOfGridPoints_
        << ",totalNumberOfBits=" << totalNumberOfBits_
        << "]";
}



}  // namespace api
}  // namespace mir

