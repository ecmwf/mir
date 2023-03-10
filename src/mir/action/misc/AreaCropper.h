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

#include "mir/action/plan/Action.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/IndexMapping.h"


namespace mir::repres {
class Representation;
}  // namespace mir::repres


namespace mir::action {


class AreaCropper : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit AreaCropper(const param::MIRParametrisation&);

    // -- Destructor

    ~AreaCropper() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    static void crop(const repres::Representation&, util::BoundingBox&, util::IndexMapping&);

    // -- Overridden methods

    void execute(context::Context&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    void print(std::ostream&) const override;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    util::BoundingBox bbox_;
    bool caching_;

    // -- Methods
    // None

    // -- Overridden methods

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
