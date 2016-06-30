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

#include "mir/style/ProdgenStyle.h"

#include <iostream>

#include "mir/param/MIRParametrisation.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/style/AutoGaussian.h"
#include "eckit/exception/Exceptions.h"
#include "mir/style/ProdgenGrid.h"

namespace mir {
namespace style {


ProdgenStyle::ProdgenStyle(const param::MIRParametrisation &parametrisation):
    ECMWFStyle(parametrisation) {

}


ProdgenStyle::~ProdgenStyle() {
}


void ProdgenStyle::print(std::ostream &out) const {
    out << "ProdgenStyle[]";
}

void ProdgenStyle::sh2grid(action::ActionPlan& plan) const {
    bool autoresol = false;
    parametrisation_.get("autoresol", autoresol);
    ASSERT(!autoresol);

    if (parametrisation_.has("user.rotation")) {

        plan.add("transform.sh2octahedral-gg",
                 "octahedral",
                 new AutoGaussian(parametrisation_));
    }
    else if (parametrisation_.has("user.grid")) {

        plan.add("transform.sh2regular-ll",
                 "grid",
                 new ProdgenGrid(parametrisation_));

        plan.add("interpolate.grid2regular-ll");

    }
    else {
        plan.add("transform.sh2octahedral-gg",
                 "octahedral",
                 new AutoGaussian(parametrisation_));
    }


    grid2grid(plan);
}


void ProdgenStyle::sh2sh(action::ActionPlan& plan) const {

    ASSERT (!parametrisation_.has("user.truncation")) ;

    bool vod2uv = false;
    parametrisation_.get("vod2uv", vod2uv);

    if (vod2uv) {
        plan.add("transform.vod2uv");
    }
}



// register MARS-specialized style
namespace {
static MIRStyleBuilder<ProdgenStyle> prodgen("prodgen");
}


}  // namespace style
}  // namespace mir

