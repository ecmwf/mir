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


#ifndef NamedGridPattern_H
#define NamedGridPattern_H

#include <string>
#include <iosfwd>

#include "eckit/utils/Regex.h"


namespace mir {
namespace namedgrids {

class NamedGrid;

class NamedGridPattern {
  public:

    // -- Exceptions
    // None

    // -- Contructors


    // -- Destructor


    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static const NamedGrid* build(const std::string &name);
    static void list(std::ostream &);


  protected:

    NamedGridPattern(const std::string &pattern);
    virtual ~NamedGridPattern(); // Change to virtual if base class


    // -- Members

    eckit::Regex pattern_;

    // -- Methods

    virtual const NamedGrid* make(const std::string &name) const = 0;
    virtual void print(std::ostream &) const = 0; // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    NamedGridPattern(const NamedGridPattern &);
    NamedGridPattern &operator=(const NamedGridPattern &);

    // -- Members
    // None

    // -- Methods


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods


    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const NamedGridPattern &p) {
        p.print(s);
        return s;
    }

};



}  // namespace logic
}  // namespace mir
#endif

