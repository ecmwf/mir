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


#include "mir/style/ProdgenStyle.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/action/transform/mapping/Mapping.h"
#include "mir/param/MIRParametrisation.h"
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

    bool vod2uv = false;
    parametrisation_.get("vod2uv", vod2uv);
    std::string transform = vod2uv? "sh-vod-to-uv-" : "sh-scalar-to-";  // completed later

    if (!parametrisation_.has("user.rotation") &&
         parametrisation_.has("user.grid")) {
        plan.add("transform." + transform + "regular-ll", "grid", new ProdgenGrid(parametrisation_));
        plan.add("interpolate.grid2regular-ll");
    }
    else {

        // use spectral mapping to cubic grid
        using namespace action::transform::mapping;
        eckit::ScopedPtr<Mapping> map(MappingFactory::build("cubic"));

        plan.add("transform." + transform + "octahedral-gg", "octahedral", map);
    }

    if (!parametrisation_.has("user.rotation")) {
        selectWindComponents(plan);
    }

    grid2grid(plan);
}


void ProdgenStyle::grid2grid(action::ActionPlan& plan) const {
    if (!parametrisation_.has("user.grid")) {
        ECMWFStyle::grid2grid(plan);
        return;
    }

    plan.add("interpolate.grid2regular-ll", "grid", new ProdgenGrid(parametrisation_));
}


namespace {
static MIRStyleBuilder<ProdgenStyle> __style("prodgen");
}


}  // namespace style
}  // namespace mir

