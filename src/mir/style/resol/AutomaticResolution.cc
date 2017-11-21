/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "mir/style/resol/AutomaticResolution.h"

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
namespace resol {


static ResolBuilder< AutomaticResolution > __resol1("automatic-resolution");
static ResolBuilder< AutomaticResolution > __resol2("auto");
static ResolBuilder< AutomaticResolution > __resol3("AUTO");


AutomaticResolution::AutomaticResolution(const param::MIRParametrisation& parametrisation) :
    Resol(parametrisation) {
}


void AutomaticResolution::prepare(action::ActionPlan& plan) const {

    // Setup spectral order mapping
    std::string order = "linear";
    parametrisation_.get("spectral-order", order);

    eckit::ScopedPtr<SpectralOrder> spectralOrder(SpectralOrderFactory::build(order));
    ASSERT(spectralOrder);

    // Set truncation
    const long N = getTargetGaussianNumber();
    ASSERT(N > 0);

    const long T = spectralOrder->getTruncationFromGaussianNumber(N);
    ASSERT(T > 0);

    long Tinput = 0;
    ASSERT(parametrisation_.fieldParametrisation().get("truncation", Tinput));
    ASSERT(Tinput > 0);

    if (Tinput > T ) {
        plan.add("transform.sh-truncate", "truncation", T);
    }


    // Set transform (TODO use a factory or better)
    std::string grid = "regular-gaussian";
    parametrisation_.get("spectral-intermediate-grid", grid);

    std::string gridname;
    if (grid == "regular-gaussian") {
        gridname = "F" + std::to_string(N);
    } else if (grid == "octahedral-gaussian") {
        gridname = "O" + std::to_string(N);
    } else if (grid == "classic-gaussian") {
        gridname = "N" + std::to_string(N);
    } else {
        throw eckit::SeriousBug("ECMWFStyle: unknown Gaussian grid '" + grid + "'");
    }

    bool vod2uv = false;
    parametrisation_.userParametrisation().get("vod2uv", vod2uv);

    if (vod2uv) {
        plan.add("transform.sh-vod-to-uv-namedgrid", "gridname", gridname);
    } else {
        plan.add("transform.sh-scalar-to-namedgrid", "gridname", gridname);
    }
}


bool AutomaticResolution::resultIsSpectral() const {
    return false;
}


void AutomaticResolution::print(std::ostream& out) const {
    out << "AutomaticResolution[]";
}


long AutomaticResolution::getTargetGaussianNumber() const {
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
    os << "ECMWFStyle: cannot calculate Gaussian number (N) from target grid";
    throw eckit::SeriousBug(os.str());
}


}  // namespace resol
}  // namespace style
}  // namespace mir

