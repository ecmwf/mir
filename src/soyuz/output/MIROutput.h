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


#ifndef MIROutput_H
#define MIROutput_H

#include <iosfwd>


class MIRParametrisation;


namespace mir {
namespace data { class MIRField; }
namespace input { class MIRInput; }
namespace output {


class MIROutput {
  public:

// -- Exceptions
    // None

// -- Contructors

    MIROutput();

// -- Destructor

    virtual ~MIROutput(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    virtual void copy(const MIRParametrisation&, input::MIRInput&) = 0; // Not iterpolation performed
    virtual void save(const MIRParametrisation&, input::MIRInput&, data::MIRField&) = 0;

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

    virtual void print(std::ostream&) const = 0; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    MIROutput(const MIROutput&);
    MIROutput& operator=(const MIROutput&);

// -- Members
    // None

// -- Methods
    // None

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend std::ostream& operator<<(std::ostream& s,const MIROutput& p) {
        p.print(s);
        return s;
    }

};


}  // namespace output
}  // namespace mir
#endif

