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

#include <memory>

#include "eckit/geo/Projection.h"
#include "eckit/geo/grid/RegularXY.h"

#include "mir/repres/Gridded.h"
#include "mir/util/Shape.h"
#include "mir/util/Types.h"


namespace mir::repres::regular {


class RegularGrid : public Gridded {
public:
    // -- Types

    using Grid       = ::eckit::geo::grid::RegularXY;
    using Projection = const ::eckit::geo::Projection;

    // -- Constructors

    RegularGrid(const param::MIRParametrisation&, Projection*);
    RegularGrid(Grid*, const util::BoundingBox&, const util::Shape&);

    RegularGrid(const RegularGrid&) = delete;
    RegularGrid(RegularGrid&&)      = delete;

    // -- Destructor

    ~RegularGrid() override;

    // -- Operators

    RegularGrid& operator=(const RegularGrid&) = delete;
    RegularGrid& operator=(RegularGrid&&)      = delete;

    // -- Methods

    static Projection* make_projection(const param::MIRParametrisation&);

    // -- Overridden methods

    Iterator* iterator() const override;

protected:
    // -- Methods

    const std::vector<double>& x() const { return grid_->x().values(); }
    const std::vector<double>& y() const { return grid_->y().values(); }
    bool xPlus() const { return grid_->x().a() < grid_->x().b(); }
    bool yPlus() const { return grid_->y().a() < grid_->y().b(); }

    const Grid& grid() const { return *grid_; }
    const util::Shape& shape() const { return shape_; }

    PointXY firstPointXY() const;
    PointLonLat firstPointLonLat() const;
    PointLonLat referencePointLonLat() const;

    // -- Overridden methods

    // from Representation
    bool sameAs(const Representation&) const override;

    bool includesNorthPole() const override;
    bool includesSouthPole() const override;
    bool isPeriodicWestEast() const override;

    void fillGrib(grib_info&) const override;
    void fillJob(api::MIRJob&) const override;
    void fillMeshGen(util::MeshGeneratorParameters&) const override;

    void reorder(long scanningMode, MIRValuesVector&) const override;
    void validate(const MIRValuesVector&) const override;
    void makeName(std::ostream&) const override;
    void print(std::ostream&) const override;

    bool extendBoundingBoxOnIntersect() const override;

    ::atlas::Grid atlasGrid() const override;
    size_t numberOfPoints() const override;

private:
    // -- Members

    std::unique_ptr<Grid> grid_;
    util::Shape shape_;
    bool firstPointBottomLeft_;
};


}  // namespace mir::repres::regular
