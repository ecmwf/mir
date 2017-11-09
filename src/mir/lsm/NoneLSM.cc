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


#include "mir/lsm/NoneLSM.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/lsm/NoMask.h"


namespace mir {
namespace lsm {


namespace {
static NoneLSM __lsm_selection("none");
}


NoneLSM::NoneLSM(const std::string &name):
    LSMSelection(name) {
}


NoneLSM::~NoneLSM() {
}


Mask& NoneLSM::noMask() {
    static NoMask none;
    return none;
}


void NoneLSM::print(std::ostream &out) const {
    out << "NoneLSM[" << name_ << "]";
}


Mask *NoneLSM::create(const param::MIRParametrisation&,
                      const repres::Representation&,
                      const std::string&) const {
    return new NoMask();
}


std::string NoneLSM::cacheKey(const param::MIRParametrisation&,
                              const repres::Representation&,
                              const std::string&) const {
    return "none";
}


}  // namespace lsm
}  // namespace mir

