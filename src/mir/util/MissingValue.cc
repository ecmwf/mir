// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/MissingValue.h"

#include <cmath>

#include "mir/param/DefaultParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::util {


double missing_value(const param::MIRParametrisation& param) {
    double missingValue = 0;
    if (!param.get("missing-value", missingValue)) {
        ASSERT(param::DefaultParametrisation::instance().get("missing-value", missingValue));
    }

    ASSERT(!std::isnan(missingValue));
    return missingValue;
}


}  // namespace mir::util
