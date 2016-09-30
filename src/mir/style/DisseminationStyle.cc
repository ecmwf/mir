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

#include "mir/param/MIRParametrisation.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/style/AutoGaussian.h"
#include "eckit/exception/Exceptions.h"

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

// if(false) {
//     if (parametrisation_.has("user.truncation")) {

// // FROM MARS STYLE: TO FACTORISE

//         if (parametrisation_.has("user.grid")) {

//             long intermediate_gaussian = 0;
//             parametrisation_.get("intermediate_gaussian", intermediate_gaussian);

//             if (intermediate_gaussian) {
//                 plan.add("transform.sh2reduced-gg", "reduced", intermediate_gaussian);
//                 plan.add("interpolate.grid2regular-ll");
//             } else {
//                 plan.add("transform.sh2regular-ll");
//             }

//             if (parametrisation_.has("user.rotation")) {
//                 plan.add("interpolate.grid2rotated-regular-ll");

//                 bool vod2uv = false;
//                 bool wind = false;

//                 parametrisation_.get("vod2uv", vod2uv);
//                 parametrisation_.get("wind", wind);

//                 if (wind || vod2uv) {
//                     plan.add("filter.adjust-winds");
//                     selectWindComponents(plan);
//                 }
//             }

//         }

//         if (parametrisation_.has("user.reduced")) {
//             plan.add("transform.sh2reduced-gg");
//         }

//         if (parametrisation_.has("user.regular")) {
//             plan.add("transform.sh2regular-gg");
//         }

//         if (parametrisation_.has("user.octahedral")) {
//             plan.add("transform.sh2octahedral-gg");
//         }

//         if (parametrisation_.has("user.pl")) {
//             plan.add("transform.sh2reduced-gg-pl-given");
//         }

//         if (parametrisation_.has("user.gridname")) {
//             std::string gridname;
//             ASSERT (parametrisation_.get("gridname", gridname));
//             plan.add("transform.sh2namedgrid");
//         }

//         if (parametrisation_.has("user.griddef")) {
//             std::string griddef;
//             ASSERT (parametrisation_.get("griddef", griddef));
//             // TODO: this is temporary
//             plan.add("transform.sh2octahedral-gg", "octahedral", 64L);
//             plan.add("interpolate.grid2griddef");
//         }
//     }
//     else {
//         plan.add("transform.sh2octahedral-gg",
//                  "octahedral", new AutoGaussian(parametrisation_));


//         grid2grid(plan);
//     }


    plan.add("transform.sh2octahedral-gg",
             "octahedral", new AutoGaussian(parametrisation_));


    grid2grid(plan);

}


void DisseminationStyle::sh2sh(action::ActionPlan& plan) const {

    // if (parametrisation_.has("user.truncation")) {
    //     plan.add("transform.sh2sh");
    // }

    bool vod2uv = false;
    parametrisation_.get("vod2uv", vod2uv);

    if (vod2uv) {
        plan.add("transform.vod2uv");
    }
}



// register MARS-specialized style
namespace {
static MIRStyleBuilder<DisseminationStyle> prodgen("dissemination");
}


}  // namespace style
}  // namespace mir

