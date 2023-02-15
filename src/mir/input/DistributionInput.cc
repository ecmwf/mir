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
#include <cmath>
#include <memory>

#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/stats/Distribution.h"
#include "mir/util/Exceptions.h"


namespace mir::input {


static const ArtificialInputBuilder<DistributionInput> __artificial("distribution");


DistributionInput::DistributionInput(const param::MIRParametrisation& /*ignored*/) {}


data::MIRField DistributionInput::field() const {
    ASSERT(dimensions() > 0);

    std::string distribution = 0;
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
