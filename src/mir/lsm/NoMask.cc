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


#include "mir/lsm/NoMask.h"

#include <ostream>

#include "mir/util/Exceptions.h"


namespace mir {
namespace lsm {


bool NoMask::active() const {
    return false;
}


bool NoMask::cacheable() const {
    return false;
}


void NoMask::hash(eckit::MD5& /*unused*/) const {
    NOTIMP;
}


const std::vector<bool>& NoMask::mask() const {
    NOTIMP;
}


void NoMask::print(std::ostream& out) const {
    out << "NoMask[]";
}

std::string NoMask::cacheName() const {
    NOTIMP;
}

}  // namespace lsm
}  // namespace mir
