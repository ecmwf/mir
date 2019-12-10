/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/input/DistributionInput.h"

#include "eckit/exception/Exceptions.h"

#include "mir/param/MIRParametrisation.h"

#include <algorithm>
#include <iostream>


namespace mir {
namespace input {


static ArtificialInputBuilder<DistributionInput> __artificial("distribution");


DistributionInput::DistributionInput(const param::MIRParametrisation& parametrisation) : ArtificialInput(parametrisation) {

    std::string distribution;
    parametrisation.get("distribution", distribution);

    distribution_.reset(stats::DistributionFactory::build(distribution));
    ASSERT(distribution_);
}


void DistributionInput::print(std::ostream& out) const {
    out << "RandomInput[distribution=" << *distribution_ << ",";
    ArtificialInput::print(out);
    out << "]";
}


data::MIRValuesVector DistributionInput::fill(size_t n) const {
    auto& dis = *distribution_;

    MIRValuesVector values(n);
    std::generate(values.begin(), values.end(), [&]() -> double { return dis(); });
    return values;
}


}  // namespace input
}  // namespace mir
