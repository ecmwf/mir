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


#ifndef DummyOutput_H
#define DummyOutput_H

#include "mir/output/MIROutput.h"


namespace eckit {
class DataHandle;
}


namespace mir {
namespace output {


class DummyOutput : public MIROutput {
  public:

// -- Exceptions
    // None

// -- Contructors

    DummyOutput();

// -- Destructor

    ~DummyOutput(); // Change to virtual if base class

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

    DummyOutput(const DummyOutput&);
    DummyOutput& operator=(const DummyOutput&);

// -- Members

// -- Methods
    // None


// -- Overridden methods
    // From MIROutput
    virtual void print(std::ostream&) const; // Change to virtual if base class
    virtual bool sameAs(const MIROutput& other) const;

    virtual size_t copy(const param::MIRParametrisation &, context::Context &); // Not iterpolation performed
    virtual size_t save(const param::MIRParametrisation&, context::Context&);


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const DummyOutput& p)
    // { p.print(s); return s; }

};


}  // namespace output
}  // namespace mir
#endif

