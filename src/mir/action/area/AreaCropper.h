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
    void estimate(context::Context&, api::MIREstimation&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action
