// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/calc/FormulaAction.h"

#include <ostream>
#include <sstream>

#include "eckit/types/Types.h"
#include "eckit/utils/Tokenizer.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Formula.h"
#include "mir/util/FormulaParser.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Translator.h"


namespace mir::action {


FormulaAction::FormulaAction(const param::MIRParametrisation& param) : Action(param) {

    std::string formula;
    ASSERT(parametrisation().get("formula", formula));

    std::string metadata;
    ASSERT(parametrisation().get("formula.metadata", metadata));

    // TODO: create a parser
    eckit::Tokenizer parse1(",");
    eckit::Tokenizer parse2("=");

    std::vector<std::string> v;
    parse1(metadata, v);

    for (auto& j : v) {
        std::vector<std::string> w;
        parse2(j, w);
        ASSERT(w.size() == 2);

        metadata_[w[0]] = util::from_string<long>(w[1]);
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
