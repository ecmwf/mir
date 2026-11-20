// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/util/Types.h"


namespace mir::param {
class MIRParametrisation;
}


namespace mir::util {


double missing_value(const param::MIRParametrisation&);


/// The given missing value, or else missing_value(), not in the values (so none is wrongly missing)
double unique_missing_value(const MIRValuesVector& values, double missingValue, const param::MIRParametrisation&);


}  // namespace mir::util
