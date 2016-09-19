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


#ifndef VOD2UVTransform_H
#define VOD2UVTransform_H

#include "mir/action/plan/Action.h"

#include <vector>

namespace mir {
namespace action {


class VOD2UVTransform : public Action {
  public:

// -- Exceptions
    // None

// -- Contructors

    VOD2UVTransform(const param::MIRParametrisation&);

// -- Destructor

    virtual ~VOD2UVTransform(); // Change to virtual if base class

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

    VOD2UVTransform(const VOD2UVTransform&);
    VOD2UVTransform& operator=(const VOD2UVTransform&);

// -- Members

    bool u_only_;
    bool v_only_;

// -- Methods


// -- Overridden methods

    virtual bool sameAs(const Action& other) const;

    virtual void execute(context::Context & ctx) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const VOD2UVTransform& p)
    //	{ p.print(s); return s; }

};


}  // namespace action
}  // namespace mir
#endif

