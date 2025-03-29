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

#include "mir/util/Rotation.h"


namespace mir::repres {


class Iterator : protected PointLatLon {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Iterator(const util::Rotation& = util::Rotation());

    Iterator(const Iterator&) = delete;
    Iterator(Iterator&&)      = delete;

    // -- Destructor

    ~Iterator() override;

    // -- Convertors

    operator bool() { return valid_; }

    // -- Operators

    void operator=(const Iterator&) = delete;
    void operator=(Iterator&&)      = delete;

    inline const PointXY& operator*() const { return pointRotated(); }

    // -- Methods

    static PointXYZ point_3D(const PointXY&);

    const PointXY& pointRotated() const;
    const PointLatLon& pointUnrotated() const;
    PointXYZ point3D() const;

    Iterator& next();
    virtual size_t index() const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    PointXY point_;
    util::Rotation rotation_;
    bool valid_;

    // -- Methods

    void print(std::ostream&) const override = 0;
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


}  // namespace mir::repres
