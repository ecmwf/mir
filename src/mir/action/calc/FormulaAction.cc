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

#include "mir/action/calc/FormulaAction.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/action/context/Context.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/FormulaParser.h"
#include "mir/util/Formula.h"
#include "mir/data/MIRField.h"

namespace mir {
namespace action {

FormulaAction::FormulaAction(const param::MIRParametrisation &parametrisation):
    Action(parametrisation),
    param_(0) {

    std::string formula;
    ASSERT(parametrisation.get("formula", formula));

    ASSERT(parametrisation.get("formula.param", param_));
    ASSERT(param_);

    std::istringstream in(formula);
    mir::util::FormulaParser p(in);
    formula_.reset(p.parse(parametrisation));
}


FormulaAction::~FormulaAction() {
}


bool FormulaAction::sameAs(const Action& other) const {
    const FormulaAction* o = dynamic_cast<const FormulaAction*>(&other);
    return o && (formula_->sameAs(*o->formula_));
}


void FormulaAction::print(std::ostream &out) const {
    out << "FormulaAction[" << *formula_ << ", param=" << param_ << "]";
}


void FormulaAction::execute(context::Context & ctx) const {

    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().calcTiming_);

    formula_->execute(ctx);

    data::MIRField& field = ctx.field();
    for (size_t i = 0; i < field.dimensions(); i++) {
        field.metadata(i, "paramId", param_);
    }

}

namespace {

static ActionBuilder< FormulaAction > formula("calc.formula");

}



}  // namespace action
}  // namespace mir

