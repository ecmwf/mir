/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "mir/style/intgrid/Automatic.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"


namespace mir {
namespace style {
namespace intgrid {


static IntgridBuilder< Automatic > __intgrid1("automatic");
static IntgridBuilder< Automatic > __intgrid2("auto");
static IntgridBuilder< Automatic > __intgrid3("AUTO");


Automatic::Automatic(const param::MIRParametrisation& parametrisation, long targetGaussianN) :
    Intgrid(parametrisation) {

    // without the target Gaussian N, don't provide an intermediate grid
    gridname_ = targetGaussianN > 0 ? ("F" + std::to_string(targetGaussianN)) : "";
}


std::string Automatic::gridname() const {
    return gridname_;
}


void Automatic::print(std::ostream& out) const {
    out << "Automatic["
        << (gridname_.empty() ? "" : "gridname=" + gridname_)
        << "]";
}


}  // namespace intgrid
}  // namespace style
}  // namespace mir

