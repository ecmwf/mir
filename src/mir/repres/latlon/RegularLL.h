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


#ifndef mir_repres_latlon_RegularLL_h
#define mir_repres_latlon_RegularLL_h

#include "mir/repres/latlon/LatLon.h"


namespace mir {
namespace repres {
namespace latlon {


class RegularLL : public LatLon {
public:

    // -- Exceptions
    // None

    // -- Contructors

    RegularLL(const param::MIRParametrisation &);
    RegularLL(const util::BoundingBox&, const util::Increments&);

    // -- Destructor

    virtual ~RegularLL();

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
    // None

    // -- Methods
    // None

    // -- Overridden methods

    Iterator* iterator() const;
    void print(std::ostream&) const;

#ifdef HAVE_ATLAS
    atlas::Grid atlasGrid() const;
#endif

    void fill(grib_info&) const;
    void fill(api::MIRJob&) const;

    void makeName(std::ostream&) const;
    bool sameAs(const Representation&) const;

    // From Representation
    const RegularLL* cropped(const util::BoundingBox&) const;

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

