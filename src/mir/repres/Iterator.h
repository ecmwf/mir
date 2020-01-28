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


#ifndef mir_repres_Iterator_h
#define mir_repres_Iterator_h

#include <iosfwd>

#include "mir/api/Atlas.h"
#include "mir/util/Rotation.h"
#include "mir/util/Types.h"


namespace mir {
namespace repres {


class Iterator : protected PointLatLon {
public:
    // -- Exceptions
    // None

    // -- Contructors

    Iterator(const util::Rotation& = util::Rotation());
    Iterator(const Iterator&) = delete;

    // -- Destructor

    virtual ~Iterator();

    // -- Convertors

    operator bool() { return valid_; }

    // -- Operators

    void operator=(const Iterator&) = delete;

    inline const Point2& operator*() const { return pointRotated(); }

    // -- Methods

    const Point2& pointRotated() const;
    const PointLatLon& pointUnrotated() const;
    const Point3 point3D() const;

    Iterator& next();

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    Point2 point_;
    atlas::util::Rotation rotation_;
    bool valid_;

    // -- Methods

    virtual void print(std::ostream&) const  = 0;
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
