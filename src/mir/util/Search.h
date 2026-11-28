// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>
#include <string>

#include "eckit/geo/Search.h"


namespace mir::param {
class MIRParametrisation;
}

namespace mir::repres {
class Representation;
}


namespace mir::util {


/// The k-d tree search of the points of the values as held (see Representation::canonicalGrid), of the k-d tree
/// storage configured (point-search-trees, or in memory if not caching), otherwise shared as of eckit::geo's
std::unique_ptr<const eckit::geo::Search> make_search(const param::MIRParametrisation&, const repres::Representation&);


/// The eckit::geo k-d tree loader of a point-search-trees value, also of former mir names
std::string search_loader(const std::string&);


}  // namespace mir::util
