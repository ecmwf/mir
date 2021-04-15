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


#include "mir/netcdf/Remapping.h"

#include <ostream>

#include "mir/netcdf/Exceptions.h"


namespace mir {
namespace netcdf {


Remapping::Remapping(size_t s) {
    reserve(s);
    for (size_t i = 0; i < s; i++) {
        push_back(i);
    }
}


Remapping::~Remapping() = default;


void Remapping::print(std::ostream& out) const {
    size_t i = 0;
    for (auto j = begin(); j != end(); ++j, ++i) {
        out << " (" << i << "->" << *j << ")";
        if (i == 9) {
            out << " ...";
            break;
        }
    }
}


void Remapping::validate() const {
    for (size_t i = 1; i < size(); i++) {
        ASSERT((*this)[i - 1] < (*this)[i]);
    }
}


}  // namespace netcdf
}  // namespace mir
