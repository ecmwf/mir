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


#include "mir/logic/AutoReduced.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/log/MIR.h"


namespace mir {
namespace logic {

// TODO: Get this list from atlas
static long RGG[] = {16, 24, 32, 48, 64, 80, 96, 128, 160, 200, 256, 320, 400,
                     512, 576, 640, 800, 1024, 1280, 1600, 2000, 4000, 8000
                    };

//==========================================================
AutoReduced::AutoReduced(const param::MIRParametrisation &parametrisation):
    parametrisation_(parametrisation) {
}


AutoReduced::~AutoReduced() {

}

void AutoReduced::get(const std::string &name, long &value) const {
    eckit::Log::trace<MIR>() << "AutoReduced::get(" << name << ")" << std::endl;
    ASSERT(name == "reduced"); // For now

    long N = 0;

    parametrisation_.get("user.regular", N);

    std::vector<double> grid;
    if (parametrisation_.get("user.grid", grid)) {
        ASSERT(grid.size() == 2);
        N = 90.0 / std::min(grid[0], grid[1]);
    }

    for (int i = sizeof(RGG) / sizeof(RGG[0]) - 1 ; i >= 0; i--) {
        if (RGG[i] >= N) {
            value = RGG[i];
        } else {
            break;
        }
    }

    if (N == 0) {
        throw eckit::SeriousBug("AutoReduced: cannot establish N");
    }

    eckit::Log::trace<MIR>() << "AutoReduced: N is " << N << ", selecting reduced N" << value << std::endl;
}

void AutoReduced::print(std::ostream &out) const {
    out << "<AutoReduced>";
}

}  // namespace param
}  // namespace mir

