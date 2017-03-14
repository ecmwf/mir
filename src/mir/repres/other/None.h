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


#ifndef None_H
#define None_H

#include "mir/repres/Representation.h"

namespace mir {
namespace repres {
namespace other {


class None : public Representation {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    None();
    None(const param::MIRParametrisation &);


    // -- Destructor

    virtual ~None(); // Change to virtual if base class

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
    // None

  protected:

    // -- Members
    // None

    // -- Methods

    void print(std::ostream &) const; // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    None(const None &);
    None &operator=(const None &);

    // -- Members

    // -- Methods
    // None


    // -- Overridden methods

    // virtual void fill(grib_info &) const;
    // virtual void fill(api::MIRJob &) const;
    // virtual atlas::grid::Grid *atlasGrid() const;
    // virtual atlas::grid::Domain domain() const;
    // virtual void validate(const std::vector<double> &values) const;
    // virtual Iterator* rotatedIterator() const; // After rotation
    // virtual Iterator* unrotatedIterator() const; // Before rotation

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const None& p)
    //  { p.print(s); return s; }

};


}  // namespace other
}  // namespace repres
}  // namespace mir
#endif

