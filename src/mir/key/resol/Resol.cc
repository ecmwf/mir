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


#include "mir/key/resol/Resol.h"

#include <algorithm>
#include <limits>
#include <ostream>

#include "mir/action/plan/ActionPlan.h"
#include "mir/key/grid/Grid.h"
#include "mir/key/intgrid/None.h"
#include "mir/key/truncation/Truncation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/SpectralOrder.h"


namespace mir::key::resol {


Resol::Resol(const param::MIRParametrisation& parametrisation, bool forceNoIntermediateGrid) :
    parametrisation_(parametrisation) {
    bool spectral = false;
    ASSERT(parametrisation_.fieldParametrisation().get("spectral", spectral) && spectral);

    // Get input truncation and a Gaussian grid number based on input (truncation) and output (grid)
    inputTruncation_ = 0;
    ASSERT(parametrisation_.fieldParametrisation().get("truncation", inputTruncation_));
    ASSERT(inputTruncation_ > 0);

    auto N = std::min(getTargetGaussianNumber(), getSourceGaussianNumber());
    ASSERT(N >= 0);

    // Setup intermediate grid (before truncation)
    // NOTE: truncation can depend on the intermediate grid Gaussian number
    if (forceNoIntermediateGrid) {
        intgrid_ = std::make_unique<intgrid::None>(parametrisation_, N);
    }
    else {
        std::string intgrid = "automatic";
        parametrisation_.get("intgrid", intgrid);
        intgrid_.reset(intgrid::IntgridFactory::build(intgrid, parametrisation_, N));
    }
    ASSERT(intgrid_);

    const auto Gi = intgrid_->gridname();
    if (!Gi.empty()) {
        N = static_cast<long>(grid::Grid::lookup(Gi, parametrisation_).gaussianNumber());
        ASSERT(N > 0);
    }

    // Setup truncation
    std::string truncation = "automatic";
    parametrisation_.userParametrisation().get("truncation", truncation);

    truncation_.reset(truncation::TruncationFactory::build(truncation, parametrisation_, N));
    ASSERT(truncation_);
}


void Resol::prepare(action::ActionPlan& plan) const {

    // truncate spectral coefficients
    long T = 0;
    if (truncation_->truncation(T, inputTruncation_)) {
        ASSERT(0 < T);
        plan.add("filter.sh-truncate", "truncation", T);
    }

    // filter(s)
    if (parametrisation_.userParametrisation().has("cesaro")) {
        plan.add("filter.sh-cesaro-summation-filter");
    }

    if (parametrisation_.userParametrisation().has("bandpass")) {
        plan.add("filter.sh-bandpass");
    }

    // transform, if specified
    const std::string grid = intgrid_->gridname();
    if (!grid.empty()) {

        bool vod2uv = false;
        parametrisation_.userParametrisation().get("vod2uv", vod2uv);

        const std::string transform =
            "transform." + std::string(vod2uv ? "sh-vod-to-uv-" : "sh-scalar-to-") + "namedgrid";
        plan.add(transform, "grid", grid);
    }
}


bool Resol::resultIsSpectral() const {
    return intgrid_->gridname().empty();
}


void Resol::print(std::ostream& out) const {
    out << "Resol[";
    const auto* sep = "";

    long T = 0;
    if (truncation_->truncation(T, inputTruncation_)) {
        out << sep << "truncation=" << T;
        sep = ",";
    }

    auto grid(gridname());
    if (!grid.empty()) {
        out << sep << "grid=" << grid;
        // sep = ",";
    }

    out << "]";
}


const std::string& Resol::gridname() const {
    return intgrid_->gridname();
}


long Resol::getTargetGaussianNumber() const {
    const auto& user = parametrisation_.userParametrisation();

    // get Gaussian N from interpreting grid
    std::string grid;
    if (grid::Grid::get("grid", grid, parametrisation_)) {
        auto N = long(grid::Grid::lookup(grid, parametrisation_).gaussianNumber());
        ASSERT(N >= 0);
        return N;
    }

    // get Gaussian N directly
    long N = std::numeric_limits<long>::max();
    if (user.get("reduced", N) || user.get("regular", N) || user.get("octahedral", N)) {
        ASSERT(N >= 0);
        return N;
    }

    // unstructured grids (hardcoded)
    if (user.has("griddef") || user.has("latitudes") || user.has("longitudes")) {
        return long(grid::Grid::default_gaussian_number());
    }

    return N;
}


long Resol::getSourceGaussianNumber() const {

    // Set Gaussian N
    std::unique_ptr<util::SpectralOrder> spectralOrder(util::SpectralOrderFactory::build("cubic"));
    ASSERT(spectralOrder);

    const long N = spectralOrder->getGaussianNumberFromTruncation(inputTruncation_);
    ASSERT(N >= 0);
    return N;
}


}  // namespace mir::key::resol
