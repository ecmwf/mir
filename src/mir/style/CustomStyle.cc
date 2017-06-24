/*
 * (C) Copyright 1996-2017 ECMWF.
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


#include "mir/style/CustomStyle.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/style/CustomParametrisation.h"


namespace mir {
namespace style {


CustomStyle::CustomStyle(const param::MIRParametrisation &parametrisation):
    MIRStyle(parametrisation) {

}


CustomStyle::~CustomStyle() {
}


void CustomStyle::prepare(action::ActionPlan &plan) const {

    plan.add("interpolate.grid2regular-ll", new CustomParametrisation(parametrisation_));
}


bool CustomStyle::forcedPrepare(const param::MIRParametrisation& parametrisation) const {
    return true;
}

void CustomStyle::print(std::ostream& out) const {
    out << "CustomStyle[]";
}


static MIRStyleBuilder<CustomStyle> __style("custom");

}  // namespace style
}  // namespace mir

