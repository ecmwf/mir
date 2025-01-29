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


#include "mir/key/grid/RegularLL.h"

#include <ostream>

#include "eckit/types/Fraction.h"
#include "eckit/utils/StringTools.h"
#include "eckit/utils/Translator.h"

#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/Exceptions.h"


namespace mir::key::grid {


RegularLL::RegularLL(const std::string& key) : Grid(key, "regular-ll") {}


util::Increments RegularLL::increments() const {
    auto grid_str = eckit::StringTools::split("/", key_);
    ASSERT_KEYWORD_GRID_SIZE(grid_str.size());

    eckit::Translator<std::string, double> cvt;
    double grid_v[2] = {cvt(grid_str[0]), cvt(grid_str[1])};

    return util::Increments(grid_v[0], grid_v[1]);
}


size_t RegularLL::gaussianNumber() const {
    auto inc = increments();
    auto r   = Latitude::GLOBE.fraction() / inc.south_north().latitude().fraction();
    auto N   = long(r.integralPart() / 2);

    ASSERT(N >= 0);
    return size_t(N);
}


const repres::Representation* RegularLL::representation() const {
    return new repres::latlon::RegularLL(increments());
}


void RegularLL::print(std::ostream& out) const {
    out << "RegularLL[key=" << key_ << "]";
}


}  // namespace mir::key::grid
