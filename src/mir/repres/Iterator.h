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
public:

    // -- Types

    struct point_ll_t {
        point_ll_t(const Latitude& latitude =0, const Longitude& longitude =0) : lat(latitude), lon(longitude) {}
        Latitude lat;
        Longitude lon;
    };

    typedef eckit::geometry::Point2 point_2d_t;
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

    const point_2d_t& operator*() const;

    // -- Methods

    const point_ll_t& pointUnrotated() const;
    const point_3d_t point3D() const;

    Iterator& next();

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members

    bool valid_;
    point_2d_t point_;
    point_ll_t pointUnrotated_;

    util::Rotation rotation_;
    atlas::Projection projection_;

    // -- Methods

    virtual void print(std::ostream&) const = 0;
    virtual bool next(Latitude&, Longitude&) = 0;

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
