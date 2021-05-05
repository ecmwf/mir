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


#include "mir/input/DistributionInput.h"

#include <algorithm>
#include <memory>
#include <ostream>

#include "mir/stats/Distribution.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace input {


static ArtificialInputBuilder<DistributionInput> __artificial("distribution");


MIRValuesVector DistributionInput::fill(size_t n) const {
    std::string distribution;
    ASSERT(parametrisation().get("distribution", distribution));

    std::unique_ptr<stats::Distribution> dis(stats::DistributionFactory::build(distribution));
    ASSERT(dis);

    MIRValuesVector values(n);
    std::generate(values.begin(), values.end(), [&]() -> double { return (*dis)(); });
    return values;
}


}  // namespace input
}  // namespace mir
