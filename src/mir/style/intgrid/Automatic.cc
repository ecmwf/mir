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
#include "mir/style/SpectralOrder.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"


namespace mir {
namespace style {
namespace intgrid {


static IntgridBuilder< Automatic > __intgrid1("automatic");
static IntgridBuilder< Automatic > __intgrid2("auto");
static IntgridBuilder< Automatic > __intgrid3("AUTO");


Automatic::Automatic(const param::MIRParametrisation& parametrisation) :
    Intgrid(parametrisation) {

    const long N = getTargetGaussianNumber();
    ASSERT(N > 0);

    gridname_ = "F" + std::to_string(N);
}


std::string Automatic::gridname() const {
    return gridname_;
}


void Automatic::print(std::ostream& out) const {
    out << "Automatic[gridname=" << gridname_ << "]";
}


long Automatic::getTargetGaussianNumber() const {
    long N = 0;

    // get N from number of points in half-meridian (uses only grid[1] South-North increment)
    std::vector<double> grid;
    if (parametrisation_.userParametrisation().get("grid", grid)) {
        ASSERT(grid.size() == 2);
        util::Increments increments(grid[0], grid[1]);

        // use (non-shifted) global bounding box
        util::BoundingBox bbox;
        increments.globaliseBoundingBox(bbox, false, false);

        N = long(increments.computeNj(bbox) - 1) / 2;
        return N;
    }

    // get Gaussian N directly
    if (parametrisation_.userParametrisation().get("reduced", N) ||
        parametrisation_.userParametrisation().get("regular", N) ||
        parametrisation_.userParametrisation().get("octahedral", N)) {
        return N;
    }

    // get Gaussian N given a gridname
    std::string gridname;
    if (parametrisation_.userParametrisation().get("gridname", gridname)) {
        N = long(namedgrids::NamedGrid::lookup(gridname).gaussianNumber());
        return N;
    }

    std::ostringstream os;
    os << "intgrid::Automatic::getTargetGaussianNumber: cannot calculate Gaussian number (N) from target grid";
    throw eckit::SeriousBug(os.str());
}


}  // namespace intgrid
}  // namespace style
}  // namespace mir

