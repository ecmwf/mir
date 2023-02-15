/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


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
