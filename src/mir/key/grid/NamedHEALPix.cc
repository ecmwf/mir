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


#include "mir/key/grid/NamedHEALPix.h"

#include <ostream>

#include "mir/repres/other/HEALPix.h"
#include "mir/util/Exceptions.h"


namespace mir::key::grid {


NamedHEALPix::NamedHEALPix(const std::string& name, size_t N) : NamedGrid(name), N_(N) {}


void NamedHEALPix::print(std::ostream& out) const {
    out << "NamedHEALPix[key=" << key_ << "]";
}


const repres::Representation* NamedHEALPix::representation() const {
    return new repres::other::HEALPix(N_);
}


const repres::Representation* NamedHEALPix::representation(const util::Rotation& /*unused*/) const {
    NOTIMP;
}


size_t NamedHEALPix::gaussianNumber() const {
    return default_gaussian_number();
}


}  // namespace mir::key::grid
