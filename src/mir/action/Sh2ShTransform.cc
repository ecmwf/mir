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

#include "mir/action/Sh2ShTransform.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"

#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace action {


Sh2ShTransform::Sh2ShTransform(const param::MIRParametrisation &parametrisation):
    Action(parametrisation),
    truncation_(0) {
    ASSERT(parametrisation.get("user.truncation", truncation_));
}


Sh2ShTransform::~Sh2ShTransform() {
}


void Sh2ShTransform::print(std::ostream &out) const {
    out << "Sh2ShTransform[";
    out << "truncation=" << truncation_;
    out << "]";
}


void Sh2ShTransform::execute(data::MIRField &field) const {
        const repres::Representation *representation = field.representation();
repres::Representation *new_representation = 0;

    for(size_t i = 0; i < field.dimensions(); i++) {
    const std::vector<double> &values = field.values(i);
    std::vector<double> result;

    repres::Representation *repres = representation->truncate(truncation_, values, result);

    if (repres) { // NULL if nothing happend
        delete new_representation;
        new_representation = repres; // Assumes representation will be the same
        field.values(result, i);
    }
}
if(new_representation) {
        field.representation(new_representation);
    }

}


namespace {
static ActionBuilder< Sh2ShTransform > subSh2ShTransform("transform.sh2sh");
}


}  // namespace action
}  // namespace mir

