// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/intgrid/None.h"


namespace mir::key::intgrid {


static const IntgridBuilder<None> __intgrid1("none");
static const IntgridBuilder<None> __intgrid2("NONE");


None::None(const param::MIRParametrisation& parametrisation, long /*unused*/) : Intgrid(parametrisation) {}


const std::string& None::gridname() const {
    static std::string empty;
    return empty;
}


}  // namespace mir::key::intgrid
