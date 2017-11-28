/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "mir/style/intgrid/None.h"

#include <iostream>


namespace mir {
namespace style {
namespace intgrid {


static IntgridBuilder< None > __intgrid1("none");
static IntgridBuilder< None > __intgrid2("NONE");


None::None(const param::MIRParametrisation& parametrisation, long) :
    style::Intgrid(parametrisation) {
}


std::string None::gridname() const {
    return std::string();
}


void None::print(std::ostream& out) const {
    out << "None[]";
}


}  // namespace intgrid
}  // namespace style
}  // namespace mir

