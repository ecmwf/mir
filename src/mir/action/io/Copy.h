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


#ifndef mir_action_io_Copy_h
#define mir_action_io_Copy_h

#include "mir/action/plan/Action.h"


namespace mir {
namespace input {
class MIRInput;
}
namespace output {
class MIROutput;
}
}


namespace mir {
namespace action {
namespace io {


class Copy : public Action {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Copy(const param::MIRParametrisation&, output::MIROutput&);

    // -- Destructor

    virtual ~Copy(); // Change to virtual if base class

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
    void custom(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // No copy allowed

    Copy(const Copy&);
    Copy &operator=(const Copy&);

    // -- Members

    output::MIROutput &output_;

    // -- Methods
    // None

    // -- Overridden methods

    virtual void execute(context::Context&) const;
    virtual bool sameAs(const Action&) const;
    virtual const char* name() const;
    virtual bool isEndAction() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const Copy& p)
    //  { p.print(s); return s; }

};


}  // namespace io
}  // namespace action
}  // namespace mir


#endif

