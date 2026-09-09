// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/key/intgrid/Intgrid.h"


namespace mir::key::intgrid {


class Automatic : public Intgrid {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Automatic(const param::MIRParametrisation& parametrisation, long targetGaussianN);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    const std::string& gridname() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::string gridname_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::key::intgrid
