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


#ifndef Iterator_H
#define Iterator_H

#include <iosfwd>
#include "mir/util/Types.h"

namespace mir {
namespace repres {


class Iterator  {
  public:


    // -- Exceptions
    // None

    // -- Contructors

    Iterator();

    // -- Destructor

    virtual ~Iterator(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual bool next(Latitude &lat, Longitude &lon) = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  protected:

    // -- Members

    // -- Methods

    virtual void print(std::ostream &) const = 0; // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    Iterator(const Iterator &);
    Iterator &operator=(const Iterator &);

    // -- Members

    // -- Methods
    // None

    // -- Overridden methods
    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const Iterator &p) {
        p.print(s);
        return s;
    }

};


}  // namespace repres
}  // namespace mir
#endif

