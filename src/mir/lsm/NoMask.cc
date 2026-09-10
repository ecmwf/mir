// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/lsm/NoMask.h"

#include <ostream>

#include "mir/util/Exceptions.h"


namespace mir::lsm {


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


}  // namespace mir::lsm
