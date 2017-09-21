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


#include "mir/style/SpectralModeTarget.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/style/SpectralOrder.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"


namespace mir {
namespace style {


namespace {
static SpectralModeBuilder<SpectralModeTarget> __spectral_mode("target");
}


SpectralModeTarget::SpectralModeTarget(const param::MIRParametrisation& parametrisation) :
    SpectralMode(parametrisation) {
}


bool SpectralModeTarget::getGridname(std::string& name) const {

    if (parametrisation_.get("spectral-grid", name)) {
        return !name.empty();
    }

    return false;
}


long SpectralModeTarget::getTruncation() const {

    // TODO: this is temporary, no support yet for unstuctured grids
    if (parametrisation_.has("griddef")) {
        return 63L;
    }

    long Tin = 0L;
    ASSERT(parametrisation_.get("field.truncation", Tin));

    std::string spectralOrder = "linear";
    parametrisation_.get("spectral-order", spectralOrder);

    eckit::ScopedPtr<SpectralOrder> order(SpectralOrderFactory::build(spectralOrder));
    ASSERT(order);

    // get truncation from points-per-latitude, limited to input
    long N = getGaussianNumberFromTarget();
    ASSERT(N > 0);

    long T = order->getTruncationFromGaussianNumber(N);
    if (T > Tin) {
        eckit::Log::warning() << "SpectralModeTarget: truncation " << T << " limited by input truncation " << Tin << std::endl;
        return Tin;
    }

    return T;
}


long SpectralModeTarget::getGaussianNumberFromTarget() const {
    long N = 0;

    // get N from number of points in half-meridian (uses only grid[1] South-North increment)
    std::vector<double> grid;
    if (parametrisation_.get("user.grid", grid)) {
        ASSERT(grid.size() == 2);
        util::Increments increments(grid[0], grid[1]);

        // use (non-shifted) global bounding box
        util::BoundingBox bbox;
        increments.globaliseBoundingBox(bbox, false, false);

        N = long(increments.computeNj(bbox) - 1) / 2;
        return N;
    }

    // get Gaussian N directly
    if (parametrisation_.get("user.reduced", N) ||
        parametrisation_.get("user.regular", N) ||
        parametrisation_.get("user.octahedral", N)) {
        return N;
    }

    // get Gaussian N given a gridname
    std::string gridname;
    if (parametrisation_.get("user.gridname", gridname)) {
        N = long(namedgrids::NamedGrid::lookup(gridname).gaussianNumber());
        return N;
    }

    std::ostringstream os;
    os << "ECMWFStyle: cannot calculate Gaussian number (N) from target grid";
    throw eckit::SeriousBug(os.str());
}


}  // namespace style
}  // namespace mir

