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
