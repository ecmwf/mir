// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/truncation/Automatic.h"

#include <memory>

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/SpectralOrder.h"


namespace mir::key::truncation {


static const TruncationBuilder<Automatic> __truncation1("automatic");
static const TruncationBuilder<Automatic> __truncation2("auto");
static const TruncationBuilder<Automatic> __truncation3("AUTO");


Automatic::Automatic(const param::MIRParametrisation& parametrisation, long targetGaussianN) :
    Truncation(parametrisation), truncation_(0) {

    // If target Gaussian N is not specified, no truncation happens
    if (targetGaussianN <= 0) {
        return;
    }

    // Setup spectral order mapping
    std::string order;
    parametrisation_.get("spectral-order", order);

    std::unique_ptr<util::SpectralOrder> spectralOrder(util::SpectralOrderFactory::build(order));
    ASSERT(spectralOrder);

    // Set truncation
    truncation_ = spectralOrder->getTruncationFromGaussianNumber(targetGaussianN);
    ASSERT(truncation_ > 0);
}


bool Automatic::truncation(long& T, long inputTruncation) const {
    ASSERT(0 < inputTruncation);
    if (0 < truncation_ && truncation_ < inputTruncation) {
        T = truncation_;
        return true;
    }
    return false;
}


}  // namespace mir::key::truncation
