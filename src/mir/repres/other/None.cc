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


#include "mir/repres/other/None.h"

#include <ostream>


namespace mir::repres::other {


None::None() = default;


None::None(const param::MIRParametrisation& /*unused*/) {}


None::~None() = default;


void None::print(std::ostream& out) const {
    out << "None[]";
}


static const RepresentationBuilder<None> builder("none");


}  // namespace mir::repres::other
