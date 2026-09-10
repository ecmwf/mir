// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/style/CustomStyle.h"

#include <fstream>
#include <ostream>
#include <sstream>

#include "mir/action/plan/ActionPlan.h"
#include "mir/output/MIROutput.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/PlanParser.h"


namespace mir::key::style {


void parse(std::istream& str, action::ActionPlan& plan, const param::MIRParametrisation& parametrisation,
           output::MIROutput& output) {

    util::PlanParser parser(str);
    parser.parse(plan, parametrisation);

    output.prepare(parametrisation, plan, output);

    ASSERT(plan.ended());
}


CustomStyle::CustomStyle(const param::MIRParametrisation& parametrisation) : MIRStyle(parametrisation) {}


CustomStyle::~CustomStyle() = default;


void CustomStyle::prepare(action::ActionPlan& plan, output::MIROutput& output) const {
    std::string s;

    if (parametrisation_.get("plan", s)) {
        std::istringstream in(s);
        parse(in, plan, parametrisation_, output);
        return;
    }

    if (parametrisation_.get("plan-script", s)) {
        std::ifstream in(s);
        if (!in) {
            throw exception::CantOpenFile(s);
        }
        parse(in, plan, parametrisation_, output);
        return;
    }

    throw exception::UserError("CustomStyle: no plan specified");
}


void CustomStyle::print(std::ostream& out) const {
    out << "CustomStyle[]";
}


static const MIRStyleBuilder<CustomStyle> __style("custom");


}  // namespace mir::key::style
