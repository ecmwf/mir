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


namespace mir {
namespace repres {
class Representation;
}
}  // namespace mir


namespace mir::action {


class Area : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit Area(const param::MIRParametrisation&);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    static void apply(const repres::Representation&, util::BoundingBox&, util::IndexMapping&, bool projection);

    const util::BoundingBox& boundingBox() const { return bbox_; }

    bool caching() const { return caching_; }

    // -- Overridden methods

    bool sameAs(const Action&) const override;

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
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action
