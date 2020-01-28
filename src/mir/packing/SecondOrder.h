/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef SecondOrder_H
#define SecondOrder_H


#include "mir/packing/Packer.h"

namespace mir {
namespace packing {


class SecondOrder : public Packer {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    SecondOrder(const std::string &name);

    // -- Destructor

    virtual ~SecondOrder(); // Change to virtual if base class

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

    SecondOrder(const SecondOrder &);
    SecondOrder &operator=(const SecondOrder &);

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

