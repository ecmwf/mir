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
/// @date Apr 2015


#ifndef soyuz_method_KNearest_H
#define soyuz_method_KNearest_H

#include "soyuz/method/MethodWeighted.h"


namespace mir {
namespace method {


class KNearest: public MethodWeighted {
  public:

// -- Exceptions
    // None

// -- Contructors
    KNearest(const param::MIRParametrisation&);

// -- Destructor
    virtual ~KNearest();

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
    // None

// -- Methods
    // None

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// -- Methods
    virtual void print(std::ostream&) const;

// -- Overridden methods
    // None

// -- Class members
    size_t nclosest_;  ///< Number of closest points to search for
    double epsilon_;

// -- Class methods
    // None

// -- Friends

    friend std::ostream& operator<<(std::ostream& s,const KNearest& p) {
        p.print(s);
        return s;
    }

};


}  // namespace method
}  // namespace mir
#endif

