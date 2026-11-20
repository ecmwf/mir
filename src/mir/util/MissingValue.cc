// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/MissingValue.h"

#include <algorithm>
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


double unique_missing_value(const MIRValuesVector& values, double missingValue,
                            const param::MIRParametrisation& param) {
    ASSERT(!values.empty());

    for (auto mv : {missingValue, missing_value(param)}) {
        if (std::find(values.begin(), values.end(), mv) == values.end()) {
            return mv;
        }
    }

    throw exception::SeriousBug("unique_missing_value: failed to get a unique missing value");
}


bool nan_to_missing(MIRValuesVector& values, double missingValue) {
    ASSERT(!std::isnan(missingValue));

    bool replaced = false;
    for (auto& v : values) {
        if (std::isnan(v)) {
            v        = missingValue;
            replaced = true;
        }
    }

    return replaced;
}


}  // namespace mir::util
