// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/data/space/Space3DVectorZonalMeridionalVertical.h"


namespace mir::data::space {


static const SpaceChoice<Space3DVectorZonalMeridionalVertical> __space1("3d-vector-u", 0, 3);
static const SpaceChoice<Space3DVectorZonalMeridionalVertical> __space2("3d-vector-v", 1, 3);
static const SpaceChoice<Space3DVectorZonalMeridionalVertical> __space3("3d-vector-w", 2, 3);


size_t Space3DVectorZonalMeridionalVertical::dimensions() const {
    return 3;
}


}  // namespace mir::data::space
