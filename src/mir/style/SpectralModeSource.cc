/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date September 2017


#include "mir/style/SpectralModeSource.h"


namespace mir {
namespace style {


namespace {
static SpectralModeBuilder<SpectralModeSource> __spectral_mode("source");
}


SpectralModeSource::SpectralModeSource(const param::MIRParametrisation& parametrisation) :
    SpectralMode(parametrisation) {
}


bool mir::style::SpectralModeSource::getGridname(std::string& name) const {

    if (parametrisation_.get("spectral-grid", name)) {
        return !name.empty();
    }

    return false;
}

long SpectralModeSource::getTruncation() const {

    // TODO: this is temporary, no support yet for unstuctured grids
    if (parametrisation_.has("griddef")) {
        return 63L;
    }

    // Set truncation if manually specified
    long T = 0;
    parametrisation_.get("user.truncation", T);

    return T;
}

}  // namespace style
}  // namespace mir

