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


#ifndef mir_action_transform_ScalarSh2NamedGrid_h
#define mir_action_transform_ScalarSh2NamedGrid_h

#include "mir/action/transform/ScalarSh2Gridded.h"


namespace mir {
namespace action {
namespace transform {


class ScalarSh2NamedGrid : public ScalarSh2Gridded {
public:

    // -- Exceptions
    // None

    // -- Contructors
    ScalarSh2NamedGrid(const param::MIRParametrisation&);

    // -- Destructor
    virtual ~ScalarSh2NamedGrid(); // Change to virtual if base class

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
    ScalarSh2NamedGrid(const ScalarSh2NamedGrid&);
    ScalarSh2NamedGrid& operator=(const ScalarSh2NamedGrid&);

    // -- Members
    std::string gridname_;

    // -- Methods
    // None

    // -- Overridden methods
    virtual bool sameAs(const Action& other) const;
    virtual const repres::Representation* outputRepresentation() const;  // from Sh2Gridded

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

