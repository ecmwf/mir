/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef soyuz_method_PseudoLaplace_H
#define soyuz_method_PseudoLaplace_H

#include "soyuz/method/KNearest.h"


namespace mir {
namespace method {


class PseudoLaplace: public KNearest {
  public:

// -- Exceptions
    // None

// -- Contructors

    PseudoLaplace(const MIRParametrisation& param);

// -- Destructor

    virtual ~PseudoLaplace();

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods
    // None

// -- Overridden methods
    virtual void assemble(MethodWeighted::Matrix& W) const;

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members


// -- Methods

    virtual void print(std::ostream&) const {}

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// -- Methods
    // None

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend std::ostream& operator<<(std::ostream& s,const PseudoLaplace& p) {
        p.print(s);
        return s;
    }

};


}  // namespace method
}  // namespace mir
#endif

