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


#ifndef mir_action_misc_SetMetadata_h
#define mir_action_misc_SetMetadata_h

#include <map>

#include "mir/action/plan/Action.h"


namespace mir {
namespace action {


class SetMetadata : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    SetMetadata(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~SetMetadata();  // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    virtual void execute(context::Context&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    std::map<std::string, long> metadata_;

    // -- Methods

    void print(std::ostream&) const;  // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // No copy allowed
    SetMetadata(const SetMetadata&);
    SetMetadata& operator=(const SetMetadata&);

    // -- Members

    // -- Methods
    // None

    // -- Overridden methods

    virtual bool sameAs(const Action&) const;
    virtual const char* name() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const SetMetadata& p)
    //  { p.print(s); return s; }
};


}  // namespace action
}  // namespace mir


#endif
