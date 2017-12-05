/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Feb 2017


#ifndef mir_action_transform_ShVodTouvRegularLL_h
#define mir_action_transform_ShVodTouvRegularLL_h

#include "mir/action/transform/ShVodTouvGridded.h"
#include "mir/util/Increments.h"


namespace mir {
namespace action {
namespace transform {


class ShVodTouvRegularLL : public ShVodTouvGridded {
public:

    // -- Exceptions
    // None

    // -- Contructors
    ShVodTouvRegularLL(const param::MIRParametrisation&);

    // -- Destructor
    virtual ~ShVodTouvRegularLL(); // Change to virtual if base class

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
    ShVodTouvRegularLL(const ShVodTouvRegularLL&);
    ShVodTouvRegularLL& operator=(const ShVodTouvRegularLL&);

    // -- Members
    util::Increments increments_;

    // -- Methods
    // None

    // -- Overridden methods
    virtual bool sameAs(const Action& other) const;
    virtual const repres::Representation* outputRepresentation() const;  // from ShToGridded
    virtual const char* name() const;

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

