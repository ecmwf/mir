/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/input/RawInput.h"

#include <ostream>

#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir::input {


RawInput::RawInput(const double* const values, size_t count, const param::MIRParametrisation& metadata) :
    values_(values), count_(count), metadata_(metadata), dimensions_(1), calls_(0) {
    ASSERT_MSG(values != nullptr, "RawInput: values != nullptr");
    ASSERT_MSG(count > 0, "RawInput: count > 0");

    long dimensions = 1;
    metadata_.get("dimensions", dimensions);
    ASSERT(dimensions >= 1);

    dimensions_ = static_cast<size_t>(dimensions);
}


bool RawInput::next() {
    return calls_++ == 0;
}


size_t RawInput::dimensions() const {
    return dimensions_;
}


const param::MIRParametrisation& RawInput::parametrisation(size_t which) const {
    ASSERT(which == 0);

    return metadata_;
}


data::MIRField RawInput::field() const {
    double missingValue = 9999.;
    bool hasMissing     = metadata_.get("missing_value", missingValue);

    // TODO support parametrisation for dimensions > 1
    data::MIRField field(parametrisation(0), hasMissing, missingValue);

    repres::RepresentationHandle repres(field.representation());
    auto n = repres->numberOfValues();
    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("RawInput", count_, n);

    const auto* here = values_;
    for (size_t which = 0; which < dimensions(); ++which, here += count_) {
        MIRValuesVector values(here, here + count_);
        field.update(values, which);
    }

    return field;
}


bool RawInput::sameAs(const MIRInput& other) const {
    if (const auto* o = dynamic_cast<const RawInput*>(&other); o != nullptr) {
        if (const auto* metadata = dynamic_cast<const param::SimpleParametrisation*>(&metadata_); metadata != nullptr) {
            return metadata->matchAll(o->metadata_);
        }
    }

    return false;
}


void RawInput::print(std::ostream& out) const {
    out << "RawInput[&values=" << values_ << ",count=" << count_ << ",metadata=" << metadata_ << "]";
}


}  // namespace mir::input
