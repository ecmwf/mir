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
#include "eckit/log/Log.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/config/LibMir.h"
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
    std::string value;

    // Get input truncation and a Gaussian grid number based on input (truncation) and output (grid)
    inputTruncation_ = 0;
    ASSERT(parametrisation_.fieldParametrisation().get("spectral", inputTruncation_));
    ASSERT(inputTruncation_ > 0);

    long N = std::min(getTargetGaussianNumber(), getSourceGaussianNumber());
    ASSERT(N >= 0);

    // Setup intermediate grid (before truncation)
    // NOTE: truncation can depend on the intermediate grid Gaussian number
    value = "automatic";
    parametrisation_.userParametrisation().get("intgrid", value);
    intgrid_.reset(IntgridFactory::build(value, parametrisation_, N));
    ASSERT(intgrid_);

    const std::string Gi = intgrid_->gridname();
    if (!Gi.empty()) {
        N = long(namedgrids::NamedGrid::lookup(Gi).gaussianNumber());
        ASSERT(N > 0);
    }

    // Setup truncation
    value = "automatic";
    parametrisation_.userParametrisation().get("truncation", value);
    truncation_.reset(TruncationFactory::build(value, parametrisation_, N));
    ASSERT(truncation_);
}


void AutomaticResolution::prepare(action::ActionPlan& plan) const {

    // truncate spectral coefficients, if specified and below input field coefficients
    long T = truncation_->truncation();
    if (0 < T && T < inputTruncation_) {
        plan.add("transform.sh-truncate", "truncation", T);
    }

    // transform, if specified
    const std::string gridname = intgrid_->gridname();
    if (!gridname.empty()) {

        bool vod2uv = false;
        parametrisation_.userParametrisation().get("vod2uv", vod2uv);

        if (vod2uv) {
            plan.add("transform.sh-vod-to-uv-namedgrid", "gridname", gridname);
        } else {
            plan.add("transform.sh-scalar-to-namedgrid", "gridname", gridname);
        }
    }
}


bool AutomaticResolution::resultIsSpectral() const {
    return intgrid_->gridname().empty();
}


void AutomaticResolution::print(std::ostream& out) const {
    out << "AutomaticResolution["
            "truncation=" << *truncation_
        << ",intgrid=" << *intgrid_
        << "]";
}


long AutomaticResolution::getTargetGaussianNumber() const {
    std::vector<double> grid;
    std::string gridname;

    long N = 0;

    if (parametrisation_.userParametrisation().get("grid", grid)) {

        // get N from number of points in half-meridian (uses only grid[1] South-North increment)
        ASSERT(grid.size() == 2);
        util::Increments increments(grid[0], grid[1]);

        // use (non-shifted) global bounding box
        util::BoundingBox bbox;
        increments.globaliseBoundingBox(bbox, false, false);

        N = long(increments.computeNj(bbox) - 1) / 2;

    } else if (parametrisation_.userParametrisation().get("reduced", N) ||
        parametrisation_.userParametrisation().get("regular", N) ||
        parametrisation_.userParametrisation().get("octahedral", N)) {

        // get Gaussian N directly

    } else if (parametrisation_.userParametrisation().get("gridname", gridname)) {

        // get Gaussian N given a gridname
        N = long(namedgrids::NamedGrid::lookup(gridname).gaussianNumber());

    } else if (parametrisation_.userParametrisation().has("griddef") ||
        parametrisation_.userParametrisation().has("points")) {

        // hardcoded
        N = 32;
        eckit::Log::debug<LibMir>() << "AutomaticResolution::getTargetGaussianNumber: setting N=" << N << " (hardcoded!)" << std::endl;

    }

    ASSERT(N >= 0);
    return N;
}


long AutomaticResolution::getSourceGaussianNumber() const {

    // Set Gaussian N
    eckit::ScopedPtr<SpectralOrder> spectralOrder(SpectralOrderFactory::build("cubic"));
    ASSERT(spectralOrder);

    const long N = spectralOrder->getGaussianNumberFromTruncation(inputTruncation_);
    ASSERT(N >= 0);
    return N;
}


}  // namespace resol
}  // namespace style
}  // namespace mir

