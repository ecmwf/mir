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


#include "mir/action/filter/ShTruncate.h"

#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/api/MIREstimation.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/util/Exceptions.h"


namespace mir::action::filter {


ShTruncate::ShTruncate(const param::MIRParametrisation& parametrisation) : Action(parametrisation), truncation_(0) {
    ASSERT(parametrisation.userParametrisation().get("truncation", truncation_));

    ASSERT(truncation_ > 0);
}


ShTruncate::~ShTruncate() = default;


bool ShTruncate::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const ShTruncate*>(&other);
    return (o != nullptr) && (truncation_ == o->truncation_);
}


void ShTruncate::print(std::ostream& out) const {
    out << "ShTruncate[" << "truncation=" << truncation_ << "]";
}


void ShTruncate::execute(context::Context& ctx) const {
    data::MIRField& field = ctx.field();

    // Keep a pointer on the original representation, as the one in the field will
    // be changed in the loop
    repres::RepresentationHandle representation(field.representation());


    for (size_t i = 0; i < field.dimensions(); i++) {
        const MIRValuesVector& values = field.values(i);
        MIRValuesVector result;

        const auto* repres = representation->truncate(truncation_, values, result);
        if (repres != nullptr) {           // NULL if nothing happend
            field.representation(repres);  // Assumes representation will be the same
            field.update(result, i);
        }
    }
}


void ShTruncate::estimate(context::Context& ctx, api::MIREstimation& estimation) const {
    data::MIRField& field = ctx.field();
    estimation.truncation(truncation_);
    field.representation(new repres::sh::SphericalHarmonics(truncation_));
}


const char* ShTruncate::name() const {
    return "ShTruncate";
}


static const ActionBuilder<ShTruncate> __action("filter.sh-truncate");


}  // namespace mir::action::filter
