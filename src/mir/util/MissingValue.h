// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/util/Types.h"


namespace mir::param {
class MIRParametrisation;
}


namespace mir::util {


double missing_value(const param::MIRParametrisation&);


}  // namespace mir::util
