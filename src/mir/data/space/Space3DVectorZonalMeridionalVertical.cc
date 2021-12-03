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


#include "mir/data/space/Space3DVectorZonalMeridionalVertical.h"


namespace mir {
namespace data {
namespace space {


static SpaceChoice<Space3DVectorZonalMeridionalVertical> __space1("3d-vector-u", 0, 3);
static SpaceChoice<Space3DVectorZonalMeridionalVertical> __space2("3d-vector-v", 1, 3);
static SpaceChoice<Space3DVectorZonalMeridionalVertical> __space3("3d-vector-w", 2, 3);


size_t Space3DVectorZonalMeridionalVertical::dimensions() const {
    return 3;
}


}  // namespace space
}  // namespace data
}  // namespace mir
