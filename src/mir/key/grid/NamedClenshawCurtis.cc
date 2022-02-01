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


#include "mir/key/grid/NamedClenshawCurtis.h"

#include <ostream>
#include <string>

#include "eckit/utils/Translator.h"

#include "mir/repres/other/ClenshawCurtis.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace key {
namespace grid {


NamedClenshawCurtis::NamedClenshawCurtis(const std::string& name) : NamedGrid(name) {}


void NamedClenshawCurtis::print(std::ostream& out) const {
    out << "NamedClenshawCurtis[key=" << key_ << "]";
}


const repres::Representation* NamedClenshawCurtis::representation() const {

    size_t N = eckit::Translator<std::string, size_t>()(key_.substr(3));
    return new repres::other::ClenshawCurtis(N);
}


const repres::Representation* NamedClenshawCurtis::representation(const util::Rotation& /*unused*/) const {
    NOTIMP;
}


size_t NamedClenshawCurtis::gaussianNumber() const {
    return 0;  // FIXME
}


}  // namespace grid
}  // namespace key
}  // namespace mir
