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

#include "soyuz/action/Sh2RegularLL.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"

#include "soyuz/repres/RegularLL.h"
#include "soyuz/param/MIRParametrisation.h"


namespace mir {
namespace action {


Sh2RegularLL::Sh2RegularLL(const param::MIRParametrisation &parametrisation):
    Sh2GriddedTransform(parametrisation) {
}


Sh2RegularLL::~Sh2RegularLL() {
}


void Sh2RegularLL::print(std::ostream &out) const {
    out << "Sh2RegularLL[]";
}


repres::Representation *Sh2RegularLL::outputRepresentation(const repres::Representation *inputRepres) const {
    eckit::Translator<std::string, double> s2d;
    std::string value;

    ASSERT(parametrisation_.get("grid", value));

    eckit::Tokenizer parse("/");

    std::vector<std::string> s;
    parse(value, s);

    ASSERT(s.size() == 2);

    double we = s2d(s[0]);
    double ns = s2d(s[1]);

    return new repres::RegularLL(util::BoundingBox(90, 0, -90, 360 - we), ns, we);
}


namespace {
static ActionBuilder< Sh2RegularLL > grid2grid("transform.sh2regular-ll");
}


}  // namespace action
}  // namespace mir

