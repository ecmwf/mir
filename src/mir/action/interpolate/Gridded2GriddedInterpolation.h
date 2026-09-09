// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>

#include "mir/action/plan/Action.h"


namespace mir {
namespace repres {
class Representation;
}
namespace method {
class Cropping;
class Method;
}  // namespace method
}  // namespace mir


namespace mir::action::interpolate {


class Gridded2GriddedInterpolation : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit Gridded2GriddedInterpolation(const param::MIRParametrisation&);

    // -- Destructor

    ~Gridded2GriddedInterpolation() override;

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

    const method::Method& method() const;
    virtual const repres::Representation* outputRepresentation() const = 0;

    // -- Overridden methods

    bool sameAs(const Action&) const override = 0;
    void print(std::ostream&) const override  = 0;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::string interpolation_;
    std::unique_ptr<method::Method> method_;
    bool inputGlobal_;
    bool outputGlobal_;

    // -- Methods

    util::BoundingBox outputBoundingBox() const override = 0;

    // -- Overridden methods

    void execute(context::Context&) const override;
    bool mergeWithNext(const Action&) override;

    bool canCrop() const override;
    method::Cropping cropping(context::Context&) const;

    bool isRegridAction() const override { return true; }

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action::interpolate
