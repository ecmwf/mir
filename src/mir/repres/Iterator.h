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


#pragma once

#include <iosfwd>

#include "eckit/geo/projection/Rotation.h"

#include "mir/util/Rotation.h"
#include "mir/util/Types.h"


namespace mir::repres {


class Iterator {
public:
    // -- Types

    using value_type = PointLonLat::value_type;

    // -- Constructors

    explicit Iterator(const util::Rotation& = util::Rotation{});

    Iterator(const Iterator&) = delete;
    Iterator(Iterator&&)      = delete;

    // -- Destructor

    virtual ~Iterator() = default;

    // -- Convertors

    operator bool() { return valid_; }

    // -- Operators

    void operator=(const Iterator&) = delete;
    void operator=(Iterator&&)      = delete;

    inline const PointLonLat& operator*() const { return pointRotated(); }

    // -- Methods

    static PointXYZ point_3d(const PointLonLat&);
    static PointLonLat point_ll(const PointXYZ&);

    const PointLonLat& pointRotated() const;
    const PointLonLat& pointUnrotated() const;
    PointXYZ point3D() const;

    Iterator& next();
    virtual size_t index() const = 0;

protected:
    // -- Members

    PointLonLat pointUnrotated_;
    PointLonLat pointRotated_;
    eckit::geo::projection::Rotation rotation_;
    bool valid_;

    // -- Methods

    virtual void print(std::ostream&) const     = 0;
    virtual bool next(value_type&, value_type&) = 0;

private:
    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Iterator& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::repres
