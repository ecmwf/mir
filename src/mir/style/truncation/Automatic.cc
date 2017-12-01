/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "mir/style/truncation/Automatic.h"

#include <algorithm>
#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/style/resol/SpectralOrder.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"


namespace mir {
namespace style {
namespace truncation {


static TruncationBuilder< Automatic > __truncation1("automatic");
static TruncationBuilder< Automatic > __truncation2("auto");
static TruncationBuilder< Automatic > __truncation3("AUTO");


Automatic::Automatic(const param::MIRParametrisation& parametrisation, long targetGaussianN) :
    style::Truncation(parametrisation)  {

    // If target Gaussian N is not specified, no truncation happens
    if (targetGaussianN == 0) {
        truncation_ = 0;
        return;
    }

    ASSERT(targetGaussianN > 0);

    // Setup spectral order mapping
    std::string order;
    parametrisation_.get("spectral-order", order);

    eckit::ScopedPtr<resol::SpectralOrder> spectralOrder(resol::SpectralOrderFactory::build(order));
    ASSERT(spectralOrder);

    // Set truncation
    truncation_ = spectralOrder->getTruncationFromGaussianNumber(targetGaussianN);
    ASSERT(truncation_ > 0);
}


long Automatic::truncation() const {
    return truncation_;
}


void Automatic::print(std::ostream& out) const {
    out << "Automatic[truncation=" << truncation_ << "]";
}


}  // namespace truncation
}  // namespace style
}  // namespace mir

