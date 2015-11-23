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


#ifndef WindInput_H
#define WindInput_H


#include "mir/input/VectorInput.h"


namespace mir {
namespace input {


class WindInput : public VectorInput {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    WindInput(MIRInput& u_component, MIRInput& v_component);

    // -- Destructor

    virtual ~WindInput(); // Change to virtual if base class

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


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    WindInput(const WindInput &);
    WindInput &operator=(const WindInput &);

    // -- Members

    // -- Methods

    // -- Overridden methods

    virtual void print(std::ostream&) const;


    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const WindInput& p)
    //  { p.print(s); return s; }

};


}  // namespace input
}  // namespace mir
#endif

