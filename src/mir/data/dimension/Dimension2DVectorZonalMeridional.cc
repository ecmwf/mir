/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/data/dimension/Dimension2DVectorZonalMeridional.h"


namespace mir {
namespace data {
namespace dimension {


static DimensionChoice<Dimension2DVectorZonalMeridional> __dimension1("2d.vector.u", 0, 2);
static DimensionChoice<Dimension2DVectorZonalMeridional> __dimension2("2d.vector.v", 1, 2);


size_t Dimension2DVectorZonalMeridional::dimensions() const {
    return 2;
}


}  // namespace dimension
}  // namespace data
}  // namespace mir

