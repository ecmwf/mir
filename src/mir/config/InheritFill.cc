/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Nov 2016


#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/config/InheritFill.h"


namespace mir {
namespace config {


InheritFill::InheritFill(const std::string& fill) : fill_(fill) {}


bool InheritFill::pick(const InheritFill* who, const std::string& fill) const {
    who = this;
    if (!fill_.length()) {
        return false;
    }

    for (std::vector< const InheritFill* >::const_iterator me=children_.begin(); me!= children_.end(); ++me) {
        ASSERT(*me != this);
        ASSERT(*me);
        if (fill == (*me)->fill_) {
            who = *me;
            return true;
        }
    }
    return false;
}


void InheritFill::inherit(param::SimpleParametrisation& who) const {
    copyValuesTo(who, false);
}


void InheritFill::print(std::ostream& out) const {
    out << "InheritFill["
        <<  "children?" << (children_.size())
        << ",empty?"    << empty()
        << ",fill="     << fill_
        << ",SimpleParametrisation[";
    SimpleParametrisation::print(out);
    out << "]]";
}


}  // namespace config
}  // namespace mir
