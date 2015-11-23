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


#ifndef NamedGrid_H
#define NamedGrid_H

#include <string>
#include <iosfwd>

namespace mir {

namespace repres {
class Representation;
}
namespace util {
class Rotation;
}
}

namespace mir {
namespace namedgrids {

class NamedGrid {
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

    virtual const repres::Representation *representation() const = 0;
    virtual const repres::Representation *representation(const util::Rotation& rotation) const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static const NamedGrid &lookup(const std::string &name);
    static void list(std::ostream &);


  protected:

    NamedGrid(const std::string &name);
    virtual ~NamedGrid(); // Change to virtual if base class


    // -- Members

    std::string name_;

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

    NamedGrid(const NamedGrid &);
    NamedGrid &operator=(const NamedGrid &);

    // -- Members
    // None

    // -- Methods


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods


    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const NamedGrid &p) {
        p.print(s);
        return s;
    }

};



}  // namespace logic
}  // namespace mir
#endif

