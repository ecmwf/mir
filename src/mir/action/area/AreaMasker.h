// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/action/area/Area.h"
#include "mir/action/plan/Action.h"


namespace mir::action {


class AreaMasker final : public Area {
public:
    // -- Exceptions
    // None

    // -- Constructors

    using Area::Area;

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    static void mask(const repres::Representation& repres, util::BoundingBox& bbox, util::IndexMapping& mapping) {
        // apply masking on projected points (lat/lon, on projection reference frame)
        AreaMasker::apply(repres, bbox, mapping, true);
    }

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void execute(context::Context&) const override;
    void print(std::ostream&) const override;
    bool sameAs(const Action&) const override;
    const char* name() const override;
    util::BoundingBox outputBoundingBox() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action
