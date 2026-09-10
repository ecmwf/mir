// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/lsm/NoneLSM.h"

#include <ostream>

#include "mir/lsm/NoMask.h"
#include "mir/util/Exceptions.h"


namespace mir::lsm {


static const NoneLSM __lsm_selection("none");


NoneLSM::NoneLSM(const std::string& name) : LSMSelection(name) {}


Mask& NoneLSM::noMask() {
    static NoMask none;
    return none;
}


void NoneLSM::print(std::ostream& out) const {
    out << "NoneLSM[" << name_ << "]";
}


Mask* NoneLSM::create(const param::MIRParametrisation& /*unused*/, const repres::Representation& /*unused*/,
                      const std::string& /*which*/) const {
    return new NoMask();
}


std::string NoneLSM::cacheKey(const param::MIRParametrisation& /*unused*/, const repres::Representation& /*unused*/,
                              const std::string& /*which*/) const {
    return "none";
}


std::string NoneLSM::cacheName() const {
    NOTIMP;
}


}  // namespace mir::lsm
