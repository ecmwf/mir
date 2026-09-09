// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/data/space/Space1DLinear.h"


namespace mir::data::space {


static const SpaceChoice<Space1DLinear> __space("1d-linear");


size_t Space1DLinear::dimensions() const {
    return 1;
}


}  // namespace mir::data::space
