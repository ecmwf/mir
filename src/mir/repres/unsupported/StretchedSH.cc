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


#include "mir/repres/unsupported/StretchedSH.h"

#include <ostream>


namespace mir {
namespace repres {


StretchedSH::StretchedSH(const param::MIRParametrisation& /*parametrisation*/) {}


void StretchedSH::print(std::ostream& out) const {
    out << "StretchedSH["
        << "]";
}


static RepresentationBuilder<StretchedSH> __repres("stretched_sh");


}  // namespace repres
}  // namespace mir
