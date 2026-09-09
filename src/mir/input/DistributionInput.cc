// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/input/DistributionInput.h"

#include <algorithm>
#include <memory>

#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/stats/Distribution.h"
#include "mir/util/Exceptions.h"


namespace mir::input {


static const ArtificialInputBuilder<DistributionInput> __artificial("distribution");


data::MIRField DistributionInput::field() const {
    ASSERT(dimensions() > 0);

    std::string distribution;
    parametrisation().get("distribution", distribution);

    data::MIRField field(parametrisation(0), false, 9999.);

    repres::RepresentationHandle repres(field.representation());
    auto n = repres->numberOfValues();
    ASSERT(n > 0);

    std::unique_ptr<stats::Distribution> dis(stats::DistributionFactory::build(distribution));
    ASSERT(dis);

    for (size_t which = 0; which < dimensions(); ++which) {

        MIRValuesVector values(n);
        std::generate(values.begin(), values.end(), [&]() -> double { return (*dis)(); });

        field.update(values, which);
    }

    return field;
}


}  // namespace mir::input
