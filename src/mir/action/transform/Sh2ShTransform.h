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


#ifndef Sh2ShTransform_H
#define Sh2ShTransform_H

#include "mir/action/Action.h"


namespace mir {
namespace action {


class Sh2ShTransform : public Action {
  public:

// -- Exceptions
    // None

// -- Contructors

    Sh2ShTransform(const param::MIRParametrisation&);

// -- Destructor

    virtual ~Sh2ShTransform(); // Change to virtual if base class

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

    Sh2ShTransform(const Sh2ShTransform&);
    Sh2ShTransform& operator=(const Sh2ShTransform&);

// -- Members

    size_t truncation_;

// -- Methods
    // None

// -- Overridden methods

    virtual void execute(data::MIRField&) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const Sh2ShTransform& p)
    //	{ p.print(s); return s; }

};


}  // namespace action
}  // namespace mir
#endif

