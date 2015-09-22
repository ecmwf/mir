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



#include "mir/namedgrids/NamedClassic.h"
#include "mir/repres/gauss/reduced/ReducedClassic.h"
#include "mir/repres/gauss/reduced/RotatedClassic.h"
#include "eckit/exception/Exceptions.h"
#include <iostream>

namespace mir {
namespace namedgrids {


NamedClassic::NamedClassic(const std::string &name, size_t N):
    NamedGrid(name),
    N_(N) {
}


NamedClassic::~NamedClassic() {
}

void NamedClassic::print(std::ostream &out) const {
    out << "NamedClassic[name=" << name_ << ",N=" << N_ << "]";
}

const repres::Representation *NamedClassic::representation() const {
    return new repres::reduced::ReducedClassic(N_);
}

const repres::Representation *NamedClassic::representation(const util::Rotation &rotation) const {
    return new repres::reduced::RotatedClassic(N_, util::BoundingBox(), rotation);
}

}  // namespace logic
}  // namespace mir

