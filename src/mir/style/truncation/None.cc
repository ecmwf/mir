/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "mir/style/truncation/None.h"

#include <iostream>

namespace mir {
namespace style {
namespace truncation {


static TruncationBuilder< None > __truncation1("none");
static TruncationBuilder< None > __truncation2("NONE");


None::None(const param::MIRParametrisation& parametrisation, long) :
    style::Truncation(parametrisation) {
}


long None::truncation() const {
    return 0;
}


void None::print(std::ostream& out) const {
    out << "None[]";
}


}  // namespace truncation
}  // namespace style
}  // namespace mir

