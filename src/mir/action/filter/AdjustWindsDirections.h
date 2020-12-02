/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_action_AdjustWindsDirections_h
#define mir_action_AdjustWindsDirections_h

#include "mir/action/plan/Action.h"
#include "mir/util/Rotation.h"


namespace mir {
namespace action {


class AdjustWindsDirections : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    AdjustWindsDirections(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~AdjustWindsDirections() override;  // Change to virtual if base class

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

    void print(std::ostream&) const;  // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    util::Rotation rotation_;

    // -- Methods
    // None

    // -- Overridden methods

    virtual void execute(context::Context& ctx) const;
    virtual bool sameAs(const Action& other) const;
    virtual const char* name() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const AdjustWinds& p)
    //	{ p.print(s); return s; }
};


}  // namespace action
}  // namespace mir


#endif
