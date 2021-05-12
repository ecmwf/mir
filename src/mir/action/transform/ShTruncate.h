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


namespace mir {
namespace action {
namespace transform {


class ShTruncate : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    ShTruncate(const param::MIRParametrisation&);

    // -- Destructor

    ~ShTruncate() override;

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

    void print(std::ostream&) const override;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    size_t truncation_;

    // -- Methods
    // None

    // -- Overridden methods

    bool sameAs(const Action&) const override;

    void execute(context::Context&) const override;

    const char* name() const override;

    void estimate(context::Context&, api::MIREstimation&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace transform
}  // namespace action
}  // namespace mir
