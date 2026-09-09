// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/intgrid/Automatic.h"


namespace mir::key::intgrid {


static const IntgridBuilder<Automatic> __intgrid1("automatic");
static const IntgridBuilder<Automatic> __intgrid2("auto");
static const IntgridBuilder<Automatic> __intgrid3("AUTO");


Automatic::Automatic(const param::MIRParametrisation& parametrisation, long targetGaussianN) :
    Intgrid(parametrisation) {

    // without the target Gaussian N, don't provide an intermediate grid
    gridname_ = targetGaussianN > 0 ? ("F" + std::to_string(targetGaussianN)) : "";
}


const std::string& Automatic::gridname() const {
    return gridname_;
}


}  // namespace mir::key::intgrid
