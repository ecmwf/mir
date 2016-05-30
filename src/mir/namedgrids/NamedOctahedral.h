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


#ifndef NamedOctahedral_H
#define NamedOctahedral_H


#include "mir/namedgrids/NamedGrid.h"

namespace mir {
namespace namedgrids {


class NamedOctahedral : public NamedGrid {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    NamedOctahedral(const std::string &name, size_t N);

    // -- Destructor

    virtual ~NamedOctahedral(); // Change to virtual if base class

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


  protected:

    // -- Members

    // -- Methods


    virtual void print(std::ostream &) const; // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    NamedOctahedral(const NamedOctahedral &);
    NamedOctahedral &operator=(const NamedOctahedral &);

    // -- Members

    size_t N_;

    // -- Methods

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    virtual const repres::Representation *representation() const;
    virtual const repres::Representation *representation(const util::Rotation& rotation) const;


    // -- Friends



};


}  // namespace namedgrids
}  // namespace mir
#endif

