// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/intgrid/NamedGrid.h"

#include "mir/util/Exceptions.h"


namespace mir::key::intgrid {


NamedGrid::NamedGrid(const std::string& gridname, const param::MIRParametrisation& parametrisation) :
    Intgrid(parametrisation), gridname_(gridname) {
    ASSERT(!gridname_.empty());
}


const std::string& NamedGrid::gridname() const {
    return gridname_;
}


}  // namespace mir::key::intgrid
