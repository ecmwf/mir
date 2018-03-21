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


#ifndef Gridded2GriddedInterpolation_H
#define Gridded2GriddedInterpolation_H

#include "mir/action/plan/Action.h"
#include "mir/method/Method.h"
#include "eckit/memory/ScopedPtr.h"

namespace mir {
namespace repres {
class Representation;
}
}


namespace mir {
namespace action {


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

    virtual void print(std::ostream&) const = 0;

    // -- Overridden methods
    // None

    virtual bool sameAs(const Action&) const = 0;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    std::string interpolation_;
    eckit::ScopedPtr<method::Method> method_;

    // -- Methods

    virtual const repres::Representation* outputRepresentation() const = 0;

    // -- Overridden methods

    virtual void execute(context::Context&) const;
    virtual bool mergeWithNext(const Action&);
    virtual bool canCrop() const;
    virtual const util::BoundingBox& croppingBoundingBox() const;
    virtual util::BoundingBox extendedBoundingBox(const util::BoundingBox&, double angle) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const Gridded2GriddedInterpolation& p)
    //  { p.print(s); return s; }

};


}  // namespace action
}  // namespace mir


#endif

