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
#include <memory>
#include <vector>

#include "eckit/maths/Matrix3.h"

#include "mir/util/Polygon2.h"
#include "mir/util/Types.h"


namespace mir::util {


class GridBox {
public:
    // -- Types

    using DualIndices = std::vector<size_t>;
    using DualPolygon = Polygon2;

    struct DualPartition {
        DualPartition(DualPolygon&& _poly, DualIndices::value_type _j, DualIndices::value_type _k) :
            poly(std::move(_poly)), j(_j), k(_k) {}

        const DualPolygon poly;
        const DualIndices::value_type j;
        const DualIndices::value_type k;
    };

    struct Dual : std::vector<DualPartition> {
        Dual(const GridBox&, const DualIndices&, const DualPolygon&);

        std::vector<DualPartition> intersect(const GridBox&) const;

        struct LatLonToStereographic {
            explicit LatLonToStereographic(Point2);
            Point2 to_stereo(Point2) const;
            Point2 to_latlon(Point2) const;

            const eckit::maths::Matrix3<double> R;
            const eckit::maths::Matrix3<double> Ri;
        } const proj;

        Point2 C;
    };

    // -- Constructors

    GridBox(double north, double west, double south, double east);

    // -- Methods

    double area() const;
    double diagonal() const;
    Point2 centre() const;

    void dual(DualIndices&&, DualPolygon&&);
    const Dual& dual() const;

    bool contains(const Point2&) const;
    bool intersects(GridBox&) const;

    double north() const { return north_; }
    double west() const { return west_; }
    double south() const { return south_; }
    double east() const { return east_; }

private:
    // -- Members

    double north_;
    double west_;
    double south_;
    double east_;

    std::shared_ptr<Dual> dual_;

    // -- Methods

    void print(std::ostream&) const;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const GridBox& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::util
