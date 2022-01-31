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


#include "mir/key/truncation/Automatic.h"

#include <memory>

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/SpectralOrder.h"


namespace mir {
namespace key {
namespace truncation {


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


}  // namespace truncation
}  // namespace key
}  // namespace mir
