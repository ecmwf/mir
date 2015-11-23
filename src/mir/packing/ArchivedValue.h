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


#ifndef ArchivedValue_H
#define ArchivedValue_H


#include "mir/packing/Packer.h"

namespace mir {
namespace packing {


class ArchivedValue : public Packer {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    ArchivedValue(const std::string &name);

    // -- Destructor

    virtual ~ArchivedValue(); // Change to virtual if base class

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

    ArchivedValue(const ArchivedValue &);
    ArchivedValue &operator=(const ArchivedValue &);

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


}  // namespace logic
}  // namespace mir
#endif

