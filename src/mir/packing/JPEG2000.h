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


#ifndef JPEG2000_H
#define JPEG2000_H


#include "mir/packing/Packer.h"

namespace mir {
namespace packing {


class JPEG2000 : public Packer {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    JPEG2000(const std::string &name);

    // -- Destructor

    virtual ~JPEG2000(); // Change to virtual if base class

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

    JPEG2000(const JPEG2000 &);
    JPEG2000 &operator=(const JPEG2000 &);

    // -- Members
    // None

    // -- Methods

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    virtual void fill(grib_info&, const repres::Representation& ) const;



    // -- Friends



};


}  // namespace packing
}  // namespace mir
#endif

