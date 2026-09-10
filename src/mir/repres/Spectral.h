// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/repres/Representation.h"
#include "mir/util/Domain.h"


namespace mir::repres {


class Spectral : public Representation {
public:
    // -- Constructors

    using Representation::Representation;

protected:
    // -- Overridden methods

    void reorder(MIRValuesVector&) const override;

    util::Domain domain() const override { return {}; }

    bool isPeriodicWestEast() const override { return true; }
    bool includesNorthPole() const override { return true; }
    bool includesSouthPole() const override { return true; }
};


}  // namespace mir::repres
