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


namespace mir::reorder {


Renumber Identity::reorder() const {
    Renumber renumber(N_);
    std::iota(renumber.begin(), renumber.end(), 0);
    return renumber;
}


static const ReorderBuilder<Identity> __reorder("identity");


}  // namespace mir::reorder
