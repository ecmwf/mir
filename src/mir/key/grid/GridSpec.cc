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


#include "mir/key/grid/GridSpec.h"

#include <ostream>

#include "mir/util/Exceptions.h"


namespace mir::key::grid {


GridSpec::GridSpec(const std::string& key) : Grid(key, "gridspec") {}


const mir::repres::Representation* GridSpec::representation() const {
    NOTIMP;
    NOTIMP;
}


const mir::repres::Representation* GridSpec::representation(const util::Rotation&) const {
    NOTIMP;
    NOTIMP;
}


const mir::repres::Representation* GridSpec::representation(const param::MIRParametrisation&) const {
    NOTIMP;
    NOTIMP;
}


size_t GridSpec::gaussianNumber() const {
    return default_gaussian_number();
}


void GridSpec::print(std::ostream& out) const {
    out << "GridSpec[key=" << key_ << "]";
}


}  // namespace mir::key::grid
