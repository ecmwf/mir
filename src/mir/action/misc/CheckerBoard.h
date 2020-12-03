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


#ifndef CheckerBoard_H
#define CheckerBoard_H

#include "mir/action/plan/Action.h"


namespace mir {
namespace action {


class CheckerBoard : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    CheckerBoard(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~CheckerBoard() override;  // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    virtual void execute(context::Context& ctx) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    void print(std::ostream&) const override;  // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // No copy allowed

    CheckerBoard(const CheckerBoard&);
    CheckerBoard& operator=(const CheckerBoard&);

    // -- Members

    // -- Methods
    // None

    // -- Overridden methods

    virtual bool sameAs(const Action& other) const override;
    virtual const char* name() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const CheckerBoard& p)
    //	{ p.print(s); return s; }
};


}  // namespace action
}  // namespace mir
#endif
