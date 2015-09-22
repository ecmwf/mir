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



#include "mir/namedgrids/NamedFull.h"
#include "mir/repres/gauss/regular/RegularGG.h"
#include "eckit/exception/Exceptions.h"
#include <iostream>

namespace mir {
namespace namedgrids {


NamedFull::NamedFull(const std::string &name, size_t N):
    NamedGrid(name),
    N_(N) {
}


NamedFull::~NamedFull() {
}

void NamedFull::print(std::ostream &out) const {
    out << "NamedFull[name=" << name_ << ",N=" << N_ << "]";
}

const repres::Representation *NamedFull::outputRepresentation(const param::MIRParametrisation &,
        const repres::Representation *inputRepres) const {
    return new repres::regular::RegularGG(N_);

}

const repres::Representation *NamedFull::outputRepresentation(const param::MIRParametrisation &,
        const repres::Representation *inputRepres,
        const util::Rotation &rotation) const {
    NOTIMP;
}

}  // namespace logic
}  // namespace mir

