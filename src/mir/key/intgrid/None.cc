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


#include "mir/key/intgrid/None.h"


namespace mir {
namespace key {
namespace intgrid {


static const IntgridBuilder<None> __intgrid1("none");
static const IntgridBuilder<None> __intgrid2("NONE");


None::None(const param::MIRParametrisation& parametrisation, long) : Intgrid(parametrisation) {}


const std::string& None::gridname() const {
    static std::string empty;
    return empty;
}


}  // namespace intgrid
}  // namespace key
}  // namespace mir
