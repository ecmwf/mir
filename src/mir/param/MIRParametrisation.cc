/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/param/MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/types/Fraction.h"

namespace mir {
namespace param {


MIRParametrisation::MIRParametrisation() {
}


MIRParametrisation::~MIRParametrisation() {
}


bool MIRParametrisation::get(const std::string &name, size_t &value) const {
    long v;
    if (get(name, v)) {
        ASSERT(v >= 0);
        value = v;
        return true;
    }
    return false;
}


bool MIRParametrisation::get(const std::string &name, std::vector<size_t> &value) const {
    std::vector<long> v;
    if (get(name, v)) {
        value.clear();
        value.reserve(v.size());
        for (const long& l : v) {
            ASSERT(l >= 0);
            value.push_back(size_t(l));
        }
        return true;
    }
    return false;
}

bool MIRParametrisation::get(const std::string& name, eckit::Fraction& value) const {
    double v;
    if(get(name, v)) {
        value = v;
        return true;
    }
    return false;
}



}  // namespace param
}  // namespace mir

