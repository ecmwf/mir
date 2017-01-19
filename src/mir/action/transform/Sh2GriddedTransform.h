/*
 * (C) Copyright 1996-2015 ECMWF.
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


#ifndef mir_action_transform_Sh2GriddedTransform_h
#define mir_action_transform_Sh2GriddedTransform_h

#include "mir/action/plan/Action.h"


namespace mir {
namespace repres {
class Representation;
}
}


namespace mir {
namespace action {
namespace transform {


class Sh2GriddedTransform : public Action {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Sh2GriddedTransform(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~Sh2GriddedTransform(); // Change to virtual if base class

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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // No copy allowed

    Sh2GriddedTransform(const Sh2GriddedTransform&);
    Sh2GriddedTransform& operator=(const Sh2GriddedTransform&);

    // -- Members
    // None

    // -- Methods

    virtual const repres::Representation* outputRepresentation() const = 0;

    // -- Overridden methods

    virtual void execute(context::Context&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const Sh2GriddedTransform& p)
    //	{ p.print(s); return s; }

};


}  // namespace transform
}  // namespace action
}  // namespace mir


#endif
