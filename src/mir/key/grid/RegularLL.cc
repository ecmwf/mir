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

#include <iostream>


namespace mir {
namespace key {
namespace grid {


RegularLL::RegularLL(const std::string& key) : Grid(key, regular_ll_t) {}


void RegularLL::print(std::ostream& out) const {
    out << "RegularLL[key=" << key_ << "]";
}


}  // namespace grid
}  // namespace key
}  // namespace mir
