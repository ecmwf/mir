/*
 * (C) Copyright 1996-2017 ECMWF.
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


#ifndef mir_repres_Iterator_h
#define mir_repres_Iterator_h

#include <iosfwd>
#include "eckit/geometry/Point3.h"
#include "eckit/memory/NonCopyable.h"
#include "atlas/projection.h"
#include "mir/util/Rotation.h"
#include "mir/util/Types.h"


namespace mir {
namespace repres {


class Iterator : private eckit::NonCopyable {
private:

    // -- Class members

    template< typename LAT, typename LON=LAT >
    struct TPoint {
        TPoint(const LAT& latitude=LAT(0), const LON& longitude=LON(0)) : lat(latitude), lon(longitude) {}
        LAT lat;
        LON lon;
    };

public:

    // -- Types

    typedef TPoint<Latitude, Longitude> point_ll_t;
    typedef TPoint<double> point_2d_t;
    typedef eckit::geometry::Point3 point_3d_t;

    // -- Exceptions
    // None

    // -- Contructors

    Iterator();
    Iterator(const util::Rotation&);

    // -- Destructor

    virtual ~Iterator();

    // -- Convertors

    operator bool() {
        return valid_;
    }

    // -- Operators
    // None

    // -- Methods

    Iterator& next();

    const point_ll_t& pointUnrotated() const;
    const point_2d_t& pointRotated() const;
    const point_3d_t point3D() const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members

    bool valid_;
    point_ll_t pointUnrotated_;
    point_2d_t pointRotated_;

    util::Rotation rotation_;
    atlas::Projection projection_;

    // -- Methods

    virtual bool next(Latitude&, Longitude&) = 0;
    virtual void print(std::ostream&) const;

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
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Iterator& p) {
        p.print(s);
        return s;
    }

};


}  // namespace repres
}  // namespace mir


#endif
