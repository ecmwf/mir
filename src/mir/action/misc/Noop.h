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


#ifndef Noop_H
#define Noop_H

#include "mir/action/plan/Action.h"


namespace mir {
namespace action {


class Noop : public Action {
public:

// -- Exceptions
    // None

// -- Contructors

    Noop(const param::MIRParametrisation&);

// -- Destructor

    virtual ~Noop(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods
    // None

// -- Overridden methods
    // None

    virtual void execute(context::Context & ctx) const;

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

    Noop(const Noop&);
    Noop& operator=(const Noop&);

// -- Members

// -- Methods
    // None

// -- Overridden methods

    virtual bool sameAs(const Action& other) const;
    virtual const char* name() const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const Noop& p)
    //  { p.print(s); return s; }

};


}  // namespace action
}  // namespace mir
#endif

