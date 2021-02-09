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


#include "mir/key/grid/NamedORCA.h"

#include <ostream>

#include "mir/repres/other/ORCA.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace key {
namespace grid {


NamedORCA::NamedORCA(const std::string& name, bool tentative) : NamedGrid(name, tentative) {}


void NamedORCA::print(std::ostream& out) const {
    out << "NamedORCA[key=" << key_ << "]";
}


const repres::Representation* NamedORCA::representation() const {
    return new repres::other::ORCA(key_);
}


const repres::Representation* NamedORCA::representation(const util::Rotation&) const {
    NOTIMP;
}


size_t NamedORCA::gaussianNumber() const {
    return defaultGaussianNumber("NamedORCA");
}


}  // namespace grid
}  // namespace key
}  // namespace mir
