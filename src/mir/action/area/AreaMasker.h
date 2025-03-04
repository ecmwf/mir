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
    void estimate(context::Context&, api::MIREstimation&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action
