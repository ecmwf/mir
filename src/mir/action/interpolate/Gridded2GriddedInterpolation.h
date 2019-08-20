/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef mir_action_interpolate_Gridded2GriddedInterpolation_h
#define mir_action_interpolate_Gridded2GriddedInterpolation_h

#include <memory>

#include "mir/action/plan/Action.h"
#include "mir/method/Method.h"


namespace mir {
namespace repres {
class Representation;
}
namespace method {
class Cropping;
}
}


namespace mir {
namespace action {
namespace interpolate {


class Gridded2GriddedInterpolation : public Action {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Gridded2GriddedInterpolation(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~Gridded2GriddedInterpolation();

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
    virtual void estimate(context::Context&, api::MIREstimation& estimation) const;

    // -- Overridden methods

    virtual bool sameAs(const Action&) const = 0;
    virtual void print(std::ostream&) const = 0;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    std::string interpolation_;
    std::unique_ptr<method::Method> method_;
    bool inputIntersectsOutput_;

    // -- Methods

    virtual util::BoundingBox outputBoundingBox() const = 0;

    // -- Overridden methods

    virtual void execute(context::Context&) const;
    virtual bool mergeWithNext(const Action&);
    virtual bool canCrop() const;

    method::Cropping cropping(context::Context& ctx) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


}  // namespace interpolate
}  // namespace action
}  // namespace mir


#endif

