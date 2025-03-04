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


#include "mir/action/calc/FormulaAction.h"

#include <ostream>
#include <sstream>

#include "eckit/types/Types.h"
#include "eckit/utils/Tokenizer.h"
#include "eckit/utils/Translator.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Formula.h"
#include "mir/util/FormulaParser.h"
#include "mir/util/MIRStatistics.h"


namespace mir::action {


FormulaAction::FormulaAction(const param::MIRParametrisation& param) : Action(param) {

    std::string formula;
    ASSERT(parametrisation().get("formula", formula));

    std::string metadata;
    ASSERT(parametrisation().get("formula.metadata", metadata));

    // TODO: create a parser
    eckit::Tokenizer parse1(",");
    eckit::Tokenizer parse2("=");
    eckit::Translator<std::string, long> s2l;

    std::vector<std::string> v;
    parse1(metadata, v);

    for (auto& j : v) {
        std::vector<std::string> w;
        parse2(j, w);
        ASSERT(w.size() == 2);

        metadata_[w[0]] = s2l(w[1]);
    }

    std::istringstream in(formula);
    util::FormulaParser p(in);
    formula_.reset(p.parse(parametrisation()));
}


bool FormulaAction::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const FormulaAction*>(&other);
    return (o != nullptr) && (formula_->sameAs(*o->formula_)) && (metadata_ == o->metadata_);
}


void FormulaAction::print(std::ostream& out) const {
    out << "FormulaAction[" << *formula_ << ", metadata=" << metadata_ << "]";
}


void FormulaAction::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().calcTimer());

    formula_->perform(ctx);

    auto& field = ctx.field();
    for (size_t i = 0; i < field.dimensions(); i++) {
        field.metadata(i, metadata_);
    }
}


const char* FormulaAction::name() const {
    return "FormulaAction";
}


static const ActionBuilder<FormulaAction> __action("calc.formula");


}  // namespace mir::action
