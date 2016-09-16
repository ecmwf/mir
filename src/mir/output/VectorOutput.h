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


#ifndef VectorOutput_H
#define VectorOutput_H

#include "mir/output/MIROutput.h"


namespace mir {
namespace output {


class VectorOutput : public MIROutput {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    VectorOutput(MIROutput &component1, MIROutput &v_component);

    // -- Destructor

    virtual ~VectorOutput(); // Change to virtual if base class

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

    MIROutput &component1_;
    MIROutput &component2_;

    // -- Methods


    // -- Overridden methods

    virtual size_t copy(const param::MIRParametrisation &, context::Context &) ; // Not iterpolation performed
    virtual size_t save(const param::MIRParametrisation &, context::Context &);
    virtual bool sameAs(const MIROutput &other) const ;
    virtual void print(std::ostream &out) const;
    virtual bool sameParametrisation(const param::MIRParametrisation &, const param::MIRParametrisation &) const;
    virtual bool printParametrisation(std::ostream& out, const param::MIRParametrisation &param) const;

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    VectorOutput(const VectorOutput &);
    VectorOutput &operator=(const VectorOutput &);

    // -- Members


    // -- Methods

    // -- Overridden methods



    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const VectorOutput& p)
    //  { p.print(s); return s; }

};


}  // namespace output
}  // namespace mir
#endif

