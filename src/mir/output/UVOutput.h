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


#ifndef UVOutput_H
#define UVOutput_H

#include "mir/output/VectorOutput.h"


namespace mir {
namespace output {


class UVOutput : public VectorOutput {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    UVOutput(MIROutput &u_component, MIROutput &v_component);

    // -- Destructor

    virtual ~UVOutput(); // Change to virtual if base class

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

    UVOutput(const UVOutput &);
    UVOutput &operator=(const UVOutput &);

    // -- Members


    // -- Methods

    // -- Overridden methods

    virtual void print(std::ostream &) const;
    virtual void save(const param::MIRParametrisation &, input::MIRInput &, data::MIRField &);

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const UVOutput& p)
    //  { p.print(s); return s; }

};


}  // namespace output
}  // namespace mir
#endif

