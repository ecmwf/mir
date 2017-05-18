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


#include "mir/style/DisseminationStyle.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/style/IntermediateGrid.h"


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

    bool vod2uv = false;
    parametrisation_.get("vod2uv", vod2uv);
    std::string transform = vod2uv? "sh-vod-to-uv-" : "sh-scalar-to-";

    // set an intermediate cubic-order octahedral Gaussian grid
    plan.add("transform." + transform + "namedgrid",
             "gridname", IntermediateGridFactory::build("octahedral", parametrisation_),
             "spectral-mapping", "cubic" );

    if (!parametrisation_.has("user.rotation")) {
        selectWindComponents(plan);
    }

    grid2grid(plan);
}


namespace {
static MIRStyleBuilder<DisseminationStyle> __style("dissemination");
}


}  // namespace style
}  // namespace mir

