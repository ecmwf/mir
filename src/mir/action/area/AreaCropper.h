// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/action/area/Area.h"
#include "mir/action/plan/Action.h"


namespace mir::action {


class AreaCropper final : public Area {
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

    static void crop(const repres::Representation& repres, util::BoundingBox& bbox, util::IndexMapping& mapping) {
        // apply cropping on non-projected points (lat/lon, on non-rotated reference frame)
        AreaCropper::apply(repres, bbox, mapping, false);
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
    bool isCropAction() const override;
    bool canCrop() const override;
    util::BoundingBox outputBoundingBox() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action
