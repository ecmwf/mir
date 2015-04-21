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


#include <iostream>

#include "eckit/exception/Exceptions.h"

#include "soyuz/action/Sh2GriddedTransform.h"


namespace mir {
namespace action {


Sh2GriddedTransform::Sh2GriddedTransform(const MIRParametrisation& parametrisation):
    Action(parametrisation) {
}


Sh2GriddedTransform::~Sh2GriddedTransform() {
}


void Sh2GriddedTransform::print(std::ostream& out) const {
    out << "Sh2GriddedTransform[]";
}


void Sh2GriddedTransform::execute(MIRField&) const {
    NOTIMP;
}


namespace {
static ActionBuilder< Sh2GriddedTransform > sh2grid("transform.sh2grid");
}


}  // namespace action
}  // namespace mir

