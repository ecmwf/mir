// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>

#include "mir/key/intgrid/Intgrid.h"


namespace mir::key::intgrid {


// Regular Gaussian grid, with the target grid Gaussian number
class RegularGGFromTargetCompatible : public Intgrid {
public:
    explicit RegularGGFromTargetCompatible(const param::MIRParametrisation&);

    const std::string& gridname() const override { return gridname_; }

protected:
    RegularGGFromTargetCompatible(const param::MIRParametrisation&, long N);

private:
    const std::string gridname_;
};


// As above, except for (non-rotated) Gaussian target grids, which need no intermediate grid
class RegularGGFromTarget : public RegularGGFromTargetCompatible {
public:
    explicit RegularGGFromTarget(const param::MIRParametrisation&);
};


}  // namespace mir::key::intgrid
