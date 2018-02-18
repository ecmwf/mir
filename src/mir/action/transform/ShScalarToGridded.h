/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Feb 2017


#ifndef mir_action_transform_ShScalarToGridded_h
#define mir_action_transform_ShScalarToGridded_h

#include "mir/action/transform/ShToGridded.h"


namespace mir {
namespace action {
namespace transform {


class ShScalarToGridded : public ShToGridded {
public:

    // -- Exceptions
    // None

    // -- Contructors
    ShScalarToGridded(const param::MIRParametrisation&);

    // -- Destructor
    virtual ~ShScalarToGridded();

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

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    void sh2grid(struct Trans_t&, data::MIRField&) const;

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
