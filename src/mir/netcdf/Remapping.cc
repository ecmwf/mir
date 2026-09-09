// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/Remapping.h"

#include <ostream>

#include "mir/netcdf/Exceptions.h"


namespace mir::netcdf {


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


}  // namespace mir::netcdf
