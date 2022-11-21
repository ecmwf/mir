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

#include <cstring>
#include <ostream>

#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace input {


RawInput::RawInput(const double* values, size_t count, const param::SimpleParametrisation& metadata) :
    values_(values), count_(count) {
    ASSERT_MSG(count > 0, "RawInput count > 0");
    ASSERT_MSG(values != nullptr, "RawInput count > 0");
    metadata.copyValuesTo(parametrisation());
}


data::MIRField RawInput::field() const {
    ASSERT(dimensions() == 1);  // TODO support dimensions > 1, missing values

    data::MIRField field(parametrisation(0), false, 9999.);

    repres::RepresentationHandle repres(field.representation());
    auto n = repres->numberOfPoints();
    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("RawInput", count_, n);

    MIRValuesVector values(values_, values_ + count_);
    field.update(values, 0, true);

    return field;
}


void RawInput::print(std::ostream& out) const {
    out << "RawInput[&values=" << values_ << ",count=" << count_ << ",metadata=" << parametrisation() << "]";
}


}  // namespace input
}  // namespace mir
