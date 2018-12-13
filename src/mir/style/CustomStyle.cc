/*
 * (C) Copyright 1996- ECMWF.
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


#include <fstream>

#include "mir/style/CustomStyle.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/PlanParser.h"


namespace mir {
namespace style {


CustomStyle::CustomStyle(const param::MIRParametrisation& parametrisation):
    MIRStyle(parametrisation) {
}


CustomStyle::~CustomStyle() = default;


void CustomStyle::prepare(action::ActionPlan& plan) const {

    std::string s;

    if (parametrisation_.get("plan", s)) {
        std::istringstream in(s);
        util::PlanParser parser(in);
        parser.parse(plan, parametrisation_);
        return;
    }

    if (parametrisation_.get("plan-script", s)) {
        std::ifstream in(s);
        if (!in) {
            throw eckit::CantOpenFile(s);
        }
        util::PlanParser parser(in);
        parser.parse(plan, parametrisation_);
        return;
    }

    throw eckit::UserError("CustomStyle: no plan specified");
}


void CustomStyle::print(std::ostream& out) const {
    out << "CustomStyle[]";
}


static MIRStyleBuilder<CustomStyle> __style("custom");


}  // namespace style
}  // namespace mir

