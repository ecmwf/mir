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


#ifndef VODInput_H
#define VODInput_H


#include "mir/input/VectorInput.h"


namespace mir {
namespace input {


class VODInput : public VectorInput {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    VODInput(MIRInput& vorticity, MIRInput& divergence);

    // -- Destructor

    virtual ~VODInput(); // Change to virtual if base class

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

    VODInput(const VODInput &);
    VODInput &operator=(const VODInput &);

    // -- Members

    // -- Methods

    // -- Overridden methods

    virtual void print(std::ostream&) const;
    virtual bool sameAs(const MIRInput& other) const;


    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const VODInput& p)
    //  { p.print(s); return s; }

};


}  // namespace input
}  // namespace mir
#endif

