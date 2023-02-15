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


#include "mir/data/space/Space1DLinear.h"


namespace mir::data::space {


static const SpaceChoice<Space1DLinear> __space("1d-linear");


size_t Space1DLinear::dimensions() const {
    return 1;
}


}  // namespace mir::data::space
