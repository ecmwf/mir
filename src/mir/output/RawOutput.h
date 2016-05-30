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


#ifndef RawOutput_H
#define RawOutput_H

#include "mir/output/MIROutput.h"


namespace eckit {
class DataHandle;
}


namespace mir {
namespace repres {
class Representation;
}

namespace output {


class RawOutput : public MIROutput {
  public:

// -- Exceptions
    // None

// -- Contructors

    RawOutput(double* values, size_t count);

// -- Destructor

    ~RawOutput(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    void shape(size_t& ni, size_t& nj) const;
    size_t size() const;

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

    RawOutput(const RawOutput&);
    RawOutput& operator=(const RawOutput&);

// -- Members

    double* values_;
    size_t count_;
    size_t size_;
    const repres::Representation* representation_;

// -- Methods
    // None


// -- Overridden methods
    // From MIROutput
    virtual void print(std::ostream&) const; // Change to virtual if base class

    virtual size_t copy(const param::MIRParametrisation&, input::MIRInput&); // Not iterpolation performed
    virtual size_t save(const param::MIRParametrisation&, input::MIRInput&, data::MIRField&);

    virtual bool sameAs(const MIROutput& other) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const RawOutput& p)
    // { p.print(s); return s; }

};


}  // namespace output
}  // namespace mir
#endif

