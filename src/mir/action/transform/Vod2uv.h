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


#ifndef mir_action_transform_Vod2uv_h
#define mir_action_transform_Vod2uv_h

#include <vector>
#include "mir/action/plan/Action.h"


namespace mir {
namespace action {
namespace transform {


class Vod2uv : public Action {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Vod2uv(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~Vod2uv(); // Change to virtual if base class

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

    Vod2uv(const Vod2uv&);
    Vod2uv& operator=(const Vod2uv&);

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    virtual bool sameAs(const Action& other) const;

    virtual void execute(context::Context & ctx) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const Vod2uv& p)
    //	{ p.print(s); return s; }

};


}  // namespace transform
}  // namespace action
}  // namespace mir


#endif

