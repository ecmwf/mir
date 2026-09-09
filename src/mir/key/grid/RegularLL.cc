// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/grid/RegularLL.h"

#include <ostream>

#include "eckit/types/Fraction.h"
#include "eckit/utils/StringTools.h"

#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Translator.h"


namespace mir::key::grid {


RegularLL::RegularLL(const std::string& key) : Grid(key, "regular-ll") {}


util::Increments RegularLL::increments() const {
    auto grid_str = eckit::StringTools::split("/", key_);
    ASSERT_KEYWORD_GRID_SIZE(grid_str.size());

    return util::Increments{util::from_string<double>(grid_str[0]), util::from_string<double>(grid_str[1])};
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
