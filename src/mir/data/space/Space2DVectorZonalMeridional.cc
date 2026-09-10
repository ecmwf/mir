// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/data/space/Space2DVectorZonalMeridional.h"


namespace mir::data::space {


static const SpaceChoice<Space2DVectorZonalMeridional> __space1("2d-vector-u", 0, 2);
static const SpaceChoice<Space2DVectorZonalMeridional> __space2("2d-vector-v", 1, 2);


size_t Space2DVectorZonalMeridional::dimensions() const {
    return 2;
}


}  // namespace mir::data::space
