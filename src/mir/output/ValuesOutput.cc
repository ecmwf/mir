// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/output/ValuesOutput.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/util/Exceptions.h"


namespace mir::output {


ValuesOutput::ValuesOutput() : missingValue_(9999), hasMissing_(false) {}


bool ValuesOutput::sameParametrisation(const param::MIRParametrisation& /*unused*/,
                                       const param::MIRParametrisation& /*unused*/) const {
    return true;
}


bool ValuesOutput::printParametrisation(std::ostream& /*unused*/, const param::MIRParametrisation& /*unused*/) const {
    return false;
}


bool ValuesOutput::sameAs(const MIROutput& other) const {
    const auto* o = dynamic_cast<const ValuesOutput*>(&other);
    return (o != nullptr) && this == o;
}


void ValuesOutput::print(std::ostream& out) const {
    out << "ValuesOutput[]";
}


size_t ValuesOutput::save(const param::MIRParametrisation& /*unused*/, context::Context& ctx) {
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


}  // namespace mir::output
