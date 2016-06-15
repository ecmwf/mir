/*
 * (C) Copyright 1996-2016 ECMWF.
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


#include "mir/repres/other/None.h"

#include <iostream>
// #include <fstream>

// #include "eckit/exception/Exceptions.h"
// #include "eckit/filesystem/PathName.h"

// #include "atlas/grid/global/Unstructured.h"

// #include "mir/param/MIRParametrisation.h"
// #include "mir/log/MIR.h"
// #include "mir/repres/Iterator.h"


namespace mir {
namespace repres {
namespace other {


None::None(const param::MIRParametrisation &parametrisation) {
}

None::None() {
}

None::~None() {
}


void None::print(std::ostream &out) const {
    out << "None[]";
}



namespace {
static RepresentationBuilder<None> builder("none");
}


}  // namespace other
}  // namespace repres
}  // namespace mir

