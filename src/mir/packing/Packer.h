/*
 * (C) Copyright 1996- ECMWF.
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


#ifndef Packer_H
#define Packer_H

#include <string>
#include <iosfwd>

struct grib_info;

namespace mir {
namespace repres {
class Representation;
}
}

namespace mir {
namespace packing {

class Packer {
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

    virtual void fill(grib_info &, const repres::Representation & ) const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static const Packer &lookup(const std::string &name);
    static void list(std::ostream &);


  protected:

    Packer(const std::string &name);
    virtual ~Packer(); // Change to virtual if base class


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

    Packer(const Packer &);
    Packer &operator=(const Packer &);

    // -- Members
    // None

    // -- Methods


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods


    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const Packer &p) {
        p.print(s);
        return s;
    }

};



}  // namespace packing
}  // namespace mir
#endif

