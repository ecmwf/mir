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


#ifndef RotatedLL_H
#define RotatedLL_H

#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/Rotation.h"


namespace mir {
namespace repres {
namespace latlon {


class RotatedLL : public RegularLL {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    RotatedLL(const param::MIRParametrisation &);
    RotatedLL(const util::BoundingBox &bbox, const util::Increments &increments, const util::Rotation &rotation);

    // -- Destructor

    virtual ~RotatedLL();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  protected:

    // -- Members

    util::Rotation rotation_;

    // -- Methods

    void print(std::ostream &) const; // Change to virtual if base class

    // -- Overridden methods
    virtual atlas::Grid atlasGrid() const;
    virtual Iterator* rotatedIterator() const;
    virtual void makeName(std::ostream&) const;
    virtual bool sameAs(const Representation& other) const;

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // RotatedLL();

    // No copy allowed

    RotatedLL(const RotatedLL &);
    RotatedLL &operator=(const RotatedLL &);

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    virtual void fill(grib_info &) const;
    virtual void fill(api::MIRJob &) const;


    // From RegularLL
    virtual const RotatedLL *cropped(const util::BoundingBox &bbox) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


}  // namespace latlon
}  // namespace repres
}  // namespace mir


#endif

