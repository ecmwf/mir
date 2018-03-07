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


#ifndef mir_action_transform_ShScalarToNamedGrid_h
#define mir_action_transform_ShScalarToNamedGrid_h

#include "mir/action/transform/ShScalarToGridded.h"


namespace mir {
namespace action {
namespace transform {


class ShScalarToNamedGrid : public ShScalarToGridded {
public:

    // -- Exceptions
    // None

    // -- Contructors

    ShScalarToNamedGrid(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~ShScalarToNamedGrid(); // Change to virtual if base class

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

    // -- Members

    std::string gridname_;

    // -- Methods
    // None

    // -- Overridden methods

    virtual bool sameAs(const Action&) const;
    virtual const char* name() const;

    virtual const repres::Representation* outputRepresentation() const;

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


#endif

