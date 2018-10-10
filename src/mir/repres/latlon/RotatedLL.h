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


#ifndef mir_repres_latlon_RotatedLL_h
#define mir_repres_latlon_RotatedLL_h

#include "mir/repres/latlon/LatLon.h"
#include "mir/util/Rotation.h"


namespace mir {
namespace repres {
namespace latlon {


class RotatedLL : public LatLon {
public:

    // -- Exceptions
    // None

    // -- Contructors

    RotatedLL(const param::MIRParametrisation&);
    RotatedLL(const util::Increments&,
              const util::Rotation&,
              const util::BoundingBox& = util::BoundingBox(),
              bool allowLatitudeShift = true,
              bool allowLongitudeShift = true);

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

private:

    // -- Members

    util::Rotation rotation_;

    // -- Methods
    // None

    // -- Overridden methods
    Iterator* iterator() const;
    void print(std::ostream&) const; // Change to virtual if base class

    atlas::Grid atlasGrid() const;

    void fill(grib_info&) const;
    void fill(api::MIRJob&) const;

    void makeName(std::ostream&) const;
    bool sameAs(const Representation&) const;

    // From Representation
    const RotatedLL* croppedRepresentation(const util::BoundingBox&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const RegularLL& p)
    //  { p.print(s); return s; }

};


}  // namespace latlon
}  // namespace repres
}  // namespace mir


#endif

