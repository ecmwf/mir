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


namespace mir {
namespace data {


MIRField::MIRField(const param::MIRParametrisation &param, bool hasMissing, double missingValue):
    values_(),
    hasMissing_(hasMissing),
    missingValue_(missingValue),
    representation_(repres::RepresentationFactory::build(param)) {
}


MIRField::MIRField(repres::Representation *repres, bool hasMissing, double missingValue):
    values_(),
    hasMissing_(hasMissing),
    missingValue_(missingValue),
    representation_(repres) {
}

// Warning: take ownership of values
void MIRField::values(std::vector<double> &values, size_t which) {
    if (values_.size() <= which) {
        values_.resize(which + 1);
    }
    std::swap(values_[which], values);
}

size_t MIRField::dimensions() const {
    return values_.size();
}


MIRField::~MIRField() {
    delete representation_;
}


void MIRField::print(std::ostream &out) const {
    out << "MIRField[dimensions=" << values_.size();
    if (hasMissing_) {
        out << ",missingValue=" << missingValue_;
    }
    if (representation_) {
        out << ",representation=" << *representation_;
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
            representation_->validate(values_[i]);
        }
    }
}

MIRField::Stats MIRField::statistics(size_t i) const
{
    const std::vector<double>& vs = values(i);
    Stats st;
    double sum = 0.;
    for(std::vector<double>::const_iterator it = vs.begin(); it != vs.end(); ++it )
    {
        double v = *it;
        st.min = std::min(v,st.min);
        st.max = std::max(v,st.max);
        sum += v;
        st.sqsum += v*v;
    }

    st.mean = sum / vs.size();
    st.stdev = std::sqrt(st.sqsum / vs.size() - st.mean * st.mean);

    return st;
}

void MIRField::representation(repres::Representation *representation) {
    delete representation_;
    representation_ = representation;
}

const std::vector<double> &MIRField::values(size_t which) const {
    ASSERT(which < values_.size());
    return values_[which];
}

std::vector<double> &MIRField::values(size_t which)  {
    ASSERT(which < values_.size());
    return values_[which];
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

void MIRField::Stats::print(std::ostream &s) const
{
    s << "Stats[min=" << min
      << ",max=" << max
      << ",l2norm=" << std::sqrt(sqsum)
      << ",mean=" << mean
      << ",stdev=" << stdev << "]";
}

}  // namespace data
}  // namespace mir

