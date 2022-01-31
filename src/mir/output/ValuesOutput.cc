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


#include "mir/output/ValuesOutput.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace output {


ValuesOutput::ValuesOutput() : missingValue_(9999), hasMissing_(false) {}


bool ValuesOutput::sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const {
    return true;
}


bool ValuesOutput::printParametrisation(std::ostream&, const param::MIRParametrisation&) const {
    return false;
}


bool ValuesOutput::sameAs(const MIROutput& other) const {
    const auto* o = dynamic_cast<const ValuesOutput*>(&other);
    return (o != nullptr) && this == o;
}


void ValuesOutput::print(std::ostream& out) const {
    out << "ValuesOutput[]";
}


size_t ValuesOutput::save(const param::MIRParametrisation&, context::Context& ctx) {
    data::MIRField& field = ctx.field();

    ASSERT(field.dimensions() == 1);

    missingValue_ = field.missingValue();
    hasMissing_   = field.hasMissing();

    values_.resize(field.dimensions());

    for (size_t i = 0; i < field.dimensions(); ++i) {
        std::swap(values_[i], field.direct(i));
    }

    return 0;
}


bool ValuesOutput::hasMissing() const {
    return hasMissing_;
}


double ValuesOutput::missingValue() const {
    return missingValue_;
}


size_t ValuesOutput::dimensions() const {
    return values_.size();
}


const MIRValuesVector& ValuesOutput::values(size_t which) const {
    ASSERT(which < values_.size());
    return values_[which];
}


}  // namespace output
}  // namespace mir
