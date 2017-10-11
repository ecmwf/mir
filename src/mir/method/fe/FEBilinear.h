/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#ifndef mir_method_fe_FEBilinear_H
#define mir_method_fe_FEBilinear_H

#include "mir/method/fe/FiniteElement.h"


namespace mir {
namespace method {
namespace fe {


class FEBilinear: public FiniteElement {
  public:

    FEBilinear(const param::MIRParametrisation&);

    virtual ~FEBilinear();

  protected:

    virtual void hash( eckit::MD5& ) const;

  private:

// -- Methods
    // None

// -- Overridden methods

    virtual void print(std::ostream&) const;
    virtual const char* name() const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

};


}  // namespace fe
}  // namespace method
}  // namespace mir

#endif

