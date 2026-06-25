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
