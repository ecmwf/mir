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


#ifndef ForwardOutput_H
#define ForwardOutput_H

#include "mir/output/MIROutput.h"


namespace mir {
namespace output {


class ForwardOutput : public MIROutput {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    ForwardOutput(MIROutput &output);

    // -- Destructor

    virtual ~ForwardOutput(); // Change to virtual if base class

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

    MIROutput &output_;

    // -- Methods


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    ForwardOutput(const ForwardOutput &);
    ForwardOutput &operator=(const ForwardOutput &);

    // -- Members


    // -- Methods

    // -- Overridden methods

    virtual size_t copy(const param::MIRParametrisation &, input::MIRInput &) ; // Not iterpolation performed
    virtual size_t save(const param::MIRParametrisation &, input::MIRInput &, data::MIRField &);

    virtual bool sameAs(const MIROutput &other) const;
    virtual void print(std::ostream &) const;
    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const ForwardOutput& p)
    //  { p.print(s); return s; }

};


}  // namespace output
}  // namespace mir
#endif

