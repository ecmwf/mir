// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <initializer_list>
#include <iosfwd>

#include "eckit/geo/projection/Rotation.h"

#include "mir/util/Rotation.h"


namespace mir::repres {


class Iterator {
public:
    // -- Constructors

    explicit Iterator(const util::Rotation& = util::Rotation());

    Iterator(const Iterator&) = delete;
    Iterator(Iterator&&)      = delete;

    // -- Destructor

    virtual ~Iterator();

    // -- Convertors

    operator bool() { return valid_; }

    // -- Operators

    void operator=(const Iterator&) = delete;
    void operator=(Iterator&&)      = delete;

    inline const PointLonLat& operator*() const { return pointRotated(); }

    // -- Methods

    static PointXYZ point_3D(const PointLonLat&);

    /// A braced list doesn't name its order (latitude/longitude or longitude/latitude), so the point is explicit
    static PointXYZ point_3D(std::initializer_list<double>) = delete;

    const PointLonLat& pointRotated() const;
    const PointLatLon& pointUnrotated() const;
    PointXYZ point3D() const;

    Iterator& next();
    virtual size_t index() const = 0;

protected:
    // -- Methods

    virtual void print(std::ostream&) const = 0;

    /// The next point (of the grid frame, unrotated), false if none
    virtual bool next(PointLonLat&) = 0;

private:
    // -- Members

    PointLatLon unrotated_;
    PointLonLat rotated_;
    eckit::geo::projection::Rotation rotation_;
    bool valid_ = true;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Iterator& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::repres
