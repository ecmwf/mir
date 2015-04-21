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


#ifndef DummyInput_H
#define DummyInput_H

#include <map>
#include <string>

#include "soyuz/input/MIRInput.h"
#include "soyuz/param/FieldParametrisation.h"


namespace mir {
namespace input {


class DummyInput : public MIRInput, public FieldParametrisation {
  public:

    // -- Exceptions
    // None

    // -- Contructors
    DummyInput();

    // -- Destructor

    virtual ~DummyInput(); // Change to virtual if base class

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

    DummyInput(const DummyInput &);
    DummyInput &operator=(const DummyInput &);

    // -- Members

    std::map<std::string, std::string> settings_;


    // -- Methods

    // -- Overridden methods
    // From MIRInput

    virtual void print(std::ostream&) const; // Change to virtual if base class

    virtual const MIRParametrisation &parametrisation() const;
    virtual MIRField *field() const;

    // From FieldParametrisation
    virtual bool lowLevelGet(const std::string&, std::string&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const DummyInput& p)
    //  { p.print(s); return s; }

};


}  // namespace input
}  // namespace mir
#endif

