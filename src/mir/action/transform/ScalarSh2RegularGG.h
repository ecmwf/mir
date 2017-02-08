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


#ifndef mir_action_transform_ScalarSh2RegularGG_h
#define mir_action_transform_ScalarSh2RegularGG_h

#include "mir/action/transform/Sh2Gridded.h"


namespace mir {
namespace action {
namespace transform {


class ScalarSh2RegularGG : public Sh2Gridded {
public:

    // -- Exceptions
    // None

    // -- Contructors

    ScalarSh2RegularGG(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~ScalarSh2RegularGG(); // Change to virtual if base class

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

    void print(std::ostream&) const; // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // No copy allowed

    ScalarSh2RegularGG(const ScalarSh2RegularGG&);
    ScalarSh2RegularGG& operator=(const ScalarSh2RegularGG&);

    // -- Members
    size_t N_;

    // -- Methods
    // None

    // -- Overridden methods

    virtual bool sameAs(const Action& other) const;

    // From Gridded2GriddedInterpolation
    virtual const repres::Representation* outputRepresentation() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const Sh2RegularGG& p)
    //	{ p.print(s); return s; }

};


}  // namespace transform
}  // namespace action
}  // namespace mir


#endif

