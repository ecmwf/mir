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


#include "mir/style/DisseminationStyle.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/style/AutoGaussian.h"


namespace mir {
namespace style {


DisseminationStyle::DisseminationStyle(const param::MIRParametrisation &parametrisation):
    ECMWFStyle(parametrisation) {

}


DisseminationStyle::~DisseminationStyle() {
}


void DisseminationStyle::print(std::ostream &out) const {
    out << "DisseminationStyle[]";
}


void DisseminationStyle::sh2grid(action::ActionPlan& plan) const {
    bool autoresol = false;
    parametrisation_.get("autoresol", autoresol);
    ASSERT(!autoresol);

    if (!parametrisation_.has("user.rotation")) {
        selectWindComponents(plan);
    }

    plan.add("transform.sh-scalar-to-octahedral-gg",
             "octahedral", new AutoGaussian(parametrisation_));


    grid2grid(plan);

}


void DisseminationStyle::shTruncate(action::ActionPlan& plan) const {
    // do nothing
}


namespace {
static MIRStyleBuilder<DisseminationStyle> __style("dissemination");
}


}  // namespace style
}  // namespace mir

