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


#include <iostream>

#include "mir/style/DisseminationStyle.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/style/AutoResol.h"
#include "mir/style/AutoReduced.h"
#include "mir/param/MIRConfiguration.h"
#include "mir/param/MIRCombinedParametrisation.h"
#include "mir/param/MIRDefaults.h"
#include "eckit/exception/Exceptions.h"
#include "mir/namedgrids/NamedGrid.h"


namespace mir {
namespace style {


DisseminationStyle::DisseminationStyle(const param::MIRParametrisation &parametrisation):
    MIRStyle(parametrisation) {

}


DisseminationStyle::~DisseminationStyle() {
}


void DisseminationStyle::print(std::ostream &out) const {
    out << "DisseminationStyle[]";
}


void DisseminationStyle::prepare(action::ActionPlan &plan) const {
    // All the nasty style goes there


    bool autoresol = false;
    bool vod2uv = false;
    bool wind = false;

    long intermediate_gaussian = 0;
    parametrisation_.get("autoresol", autoresol);
    parametrisation_.get("vod2uv", vod2uv);
    parametrisation_.get("wind", wind);

    parametrisation_.get("intermediate_gaussian", intermediate_gaussian);

    bool user_grid = parametrisation_.has("user.grid");
    bool user_reduced = parametrisation_.has("user.reduced");
    bool user_regular = parametrisation_.has("user.regular");
    bool user_octahedral = parametrisation_.has("user.octahedral");
    bool user_pl = parametrisation_.has("user.pl");
    bool user_gridname = parametrisation_.has("user.gridname");

    if (user_grid) {
        ASSERT(!user_reduced);
        ASSERT(!user_regular);
        ASSERT(!user_octahedral);
        ASSERT(!user_pl);
        ASSERT(!user_gridname);
    }

    if (user_reduced) {
        ASSERT(!user_grid);
        ASSERT(!user_regular);
        ASSERT(!user_octahedral);
        ASSERT(!user_pl);
        ASSERT(!user_gridname);
    }

    if (user_regular) {
        ASSERT(!user_grid);
        ASSERT(!user_reduced);
        ASSERT(!user_octahedral);
        ASSERT(!user_pl);
        ASSERT(!user_gridname);
    }

    if (user_octahedral) {
        ASSERT(!user_grid);
        ASSERT(!user_reduced);
        ASSERT(!user_regular);
        ASSERT(!user_pl);
        ASSERT(!user_gridname);
    }

    if (user_pl) {
        ASSERT(!user_grid);
        ASSERT(!user_reduced);
        ASSERT(!user_regular);
        ASSERT(!user_octahedral);
        ASSERT(!user_gridname);
    }

    if (user_gridname) {
        ASSERT(!user_grid);
        ASSERT(!user_reduced);
        ASSERT(!user_regular);
        ASSERT(!user_octahedral);
        ASSERT(!user_pl);
    }

    bool field_gridded = parametrisation_.has("field.gridded");

    if (parametrisation_.has("field.spectral")) {
        if (parametrisation_.has("user.truncation")) {
            plan.add("transform.sh2sh");
        }

        if (vod2uv) {
            plan.add("transform.vod2uv");
        }


        if (user_grid || user_reduced || user_regular || user_octahedral || user_pl || user_gridname) {
            // if(autoresol) {
            //     plan.add("transform.sh2sh", "truncation", new AutoResol(parametrisation_));
            // }

            plan.add("transform.sh2octahedral-gg", "octahedral", 1280);
            field_gridded = true;
        }
    }


    if (field_gridded) {

        if (user_grid) {
            if (parametrisation_.has("user.rotation")) {
                plan.add("interpolate.grid2rotated-regular-ll");
                if (wind || vod2uv) {
                    plan.add("filter.adjust-winds");
                }
            } else {
                plan.add("interpolate.grid2regular-ll");
            }
        }

        if (user_reduced) {
            if (parametrisation_.has("user.rotation")) {
                plan.add("interpolate.grid2rotated-reduced-gg");
                if (wind || vod2uv) {
                    plan.add("filter.adjust-winds");
                }
            } else {
                plan.add("interpolate.grid2reduced-gg");
            }
        }

        if (user_regular) {
            if (parametrisation_.has("user.rotation")) {
                plan.add("interpolate.grid2rotated-regular-gg");
                if (wind || vod2uv) {
                    plan.add("filter.adjust-winds");
                }
            } else {
                plan.add("interpolate.grid2regular-gg");
            }
        }

        if (user_octahedral) {
            if (parametrisation_.has("user.rotation")) {
                plan.add("interpolate.grid2rotated-octahedral-gg");
                if (wind || vod2uv) {
                    plan.add("filter.adjust-winds");
                }
            } else {
                plan.add("interpolate.grid2octahedral-gg");
            }
        }

        if (user_pl) {
            ASSERT(!parametrisation_.has("user.rotation"));
            plan.add("interpolate.grid2reduced-gg-pl-given");
        }

        if (user_gridname) {
            std::string gridname;
            ASSERT (parametrisation_.get("gridname", gridname));

            if (parametrisation_.has("user.rotation")) {
                plan.add("interpolate.grid2rotated-namedgrid");
                if (wind || vod2uv) {
                    plan.add("filter.adjust-winds");
                }
            } else {
                plan.add("interpolate.grid2namedgrid");
            }
        }

    }

    if (parametrisation_.has("user.area")) {
        plan.add("crop.area");
    }

    if (parametrisation_.has("user.bitmap")) {
        plan.add("filter.bitmap");
    }

    if (parametrisation_.has("user.frame")) {
        plan.add("filter.frame");
    }
}


// register MARS-specialized style
namespace {
static MIRStyleBuilder<DisseminationStyle> prodgen("dissemination");
}


}  // namespace style
}  // namespace mir

