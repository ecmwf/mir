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

#include "soyuz/logic/MARSLogic.h"
#include "soyuz/param/MIRParametrisation.h"


namespace mir {
namespace logic {


MARSLogic::MARSLogic(const param::MIRParametrisation &parametrisation):
    MIRLogic(parametrisation) {

}


MARSLogic::~MARSLogic() {
}


void MARSLogic::print(std::ostream &out) const {
    out << "MARSLogic[]";
}


void MARSLogic::prepare(std::vector<std::auto_ptr< action::Action > > &actions) const {
    // All the nasty logic goes there

    if (parametrisation_.has("field.spherical")) {
        if (parametrisation_.has("user.truncation")) {
            add(actions, "transform.sh2sh");
        }
    }

    if (parametrisation_.has("field.spherical")) {
        if (parametrisation_.has("user.grid")) {
#if 0
            add(actions, "transform.sh2regular-ll");
#else
            // For now, thar's what we do
            add(actions, "transform.sh2reduced-gg");
            add(actions, "interpolate.grid2regular-ll");
#endif
        }
        if (parametrisation_.has("user.reduced")) {
            add(actions, "transform.sh2reduced-gg");

        }
        if (parametrisation_.has("user.regular")) {
            add(actions, "transform.sh2regular-gg");
        }
    }

    if (parametrisation_.has("field.gridded")) {
        if (parametrisation_.has("user.grid")) {
            add(actions, "interpolate.grid2regular-ll");
        }
        if (parametrisation_.has("user.reduced")) {
            add(actions, "interpolate.grid2reduced-gg");
        }
        if (parametrisation_.has("user.regular")) {
            add(actions, "interpolate.grid2regular-gg");
        }
    }

    if (parametrisation_.has("user.area")) {
        add(actions, "crop.area");
    }

    if (parametrisation_.has("user.bitmap")) {
        add(actions, "filter.bitmap");
    }

    if (parametrisation_.has("user.frame")) {
        add(actions, "filter.frame");
    }
}


// register MARS-specialized logic
namespace {
static MIRLogicBuilder<MARSLogic> mars("mars");
}


}  // namespace logic
}  // namespace mir

