// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/data/space/Space3DCartesian.h"


namespace mir::data::space {


static const SpaceChoice<Space3DCartesian> __space1("3d-cartesian-x", 0, 3);
static const SpaceChoice<Space3DCartesian> __space2("3d-cartesian-y", 1, 3);
static const SpaceChoice<Space3DCartesian> __space3("3d-cartesian-z", 2, 3);


size_t Space3DCartesian::dimensions() const {
    return 3;
}


}  // namespace mir::data::space
