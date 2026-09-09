// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>

#include "eckit/geo/projection/Rotation.h"

#include "mir/util/Rotation.h"


namespace mir::repres {


class Iterator : protected PointLatLon {
public:
    // -- Constructors

    explicit Iterator(const util::Rotation& = util::Rotation());

    Iterator(const Iterator&) = delete;
    Iterator(Iterator&&)      = delete;

    // -- Destructor

    ~Iterator() override;

    // -- Convertors

    operator bool() { return valid_; }

    // -- Operators

    void operator=(const Iterator&) = delete;
    void operator=(Iterator&&)      = delete;

    inline const Point2& operator*() const { return pointRotated(); }

    // -- Methods

    static Point3 point_3D(const Point2&);

    const Point2& pointRotated() const;
    const PointLatLon& pointUnrotated() const;
    Point3 point3D() const;

    Iterator& next();
    virtual size_t index() const = 0;

protected:
    // -- Members

    Point2 point_;
    eckit::geo::projection::Rotation rotation_;
    bool valid_;

    // -- Methods

    void print(std::ostream&) const override = 0;
    virtual bool next(Latitude&, Longitude&) = 0;

private:
    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Iterator& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::repres
