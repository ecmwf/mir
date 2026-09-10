// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/key/truncation/Truncation.h"


namespace mir::key::truncation {


class Automatic : public Truncation {
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

    bool truncation(long&, long inputTruncation) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    long truncation_;

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


}  // namespace mir::key::truncation
