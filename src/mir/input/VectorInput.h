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


#ifndef VectorInput_H
#define VectorInput_H


#include "eckit/io/Buffer.h"

#include "mir/input/MIRInput.h"


namespace mir {
namespace input {


class VectorInput : public MIRInput {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    VectorInput(MIRInput& component1, MIRInput& component2);

    // -- Destructor

    virtual ~VectorInput(); // Change to virtual if base class

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
    MIRInput& component1_;
    MIRInput& component2_;

    // -- Methods


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    VectorInput(const VectorInput &);
    VectorInput &operator=(const VectorInput &);

    // -- Members


    // -- Methods

    // -- Overridden methods

    virtual const param::MIRParametrisation& parametrisation() const;
    virtual data::MIRField* field() const;
    virtual grib_handle* gribHandle() const;
    virtual bool next();


    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const VectorInput& p)
    //  { p.print(s); return s; }

};


}  // namespace input
}  // namespace mir
#endif

