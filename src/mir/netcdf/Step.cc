// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/Step.h"


namespace mir::netcdf {


bool Step::merge(Step* /*other*/) {
    return false;
}


}  // namespace mir::netcdf
