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
#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"

#include "soyuz/data/MIRField.h"
#include "soyuz/param/MIRParametrisation.h"
#include "soyuz/repres/Representation.h"

#include "soyuz/action/Sh2ShTransform.h"


namespace mir {
namespace action {


Sh2ShTransform::Sh2ShTransform(const param::MIRParametrisation &parametrisation):
    Action(parametrisation),
    truncation_(0) {
    std::string value;
    ASSERT(parametrisation.get("user.truncation", value));

    truncation_ = eckit::Translator<std::string, size_t>()(value);
}


Sh2ShTransform::~Sh2ShTransform() {
}


void Sh2ShTransform::print(std::ostream &out) const {
    out << "Sh2ShTransform[";
    out << "truncation=" << truncation_;
    out << "]";
}


void Sh2ShTransform::execute(data::MIRField &field) const {
    const std::vector<double> &values = field.values();
    std::vector<double> result;

    const repres::Representation *representation = field.representation();
    repres::Representation *repres = representation->truncate(truncation_, values, result);

    if (repres) { // NULL if nothing happend
        field.representation(repres);
        field.values(result);
    }
}


namespace {
static ActionBuilder< Sh2ShTransform > subSh2ShTransform("transform.sh2sh");
}


}  // namespace action
}  // namespace mir

