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


#include "mir/reorder/Identity.h"

#include <numeric>
#include <ostream>


namespace mir::reorder {


Renumber Identity::reorder(size_t N) const {
    Renumber renumber(N);
    std::iota(renumber.begin(), renumber.end(), 0);
    return renumber;
}


void Identity::print(std::ostream &s) const {
    s << "Identity[]";
}


static const ReorderBuilder<Identity> __reorder("identity");


}  // namespace mir::reorder
