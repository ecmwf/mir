// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <memory>

#include "mir/repres/Gridded.h"
#include "mir/util/Types.h"


namespace eckit::geo {
class Grid;
}


namespace mir::repres {


/// Regular grid in projected coordinates (eckit::geo regular_xy), by gridSpec or by GRIB gridType keys
class RegularXY : public Gridded {
public:
    // -- Constructors

    explicit RegularXY(const param::MIRParametrisation&);

    // -- Destructor

    ~RegularXY() override;

protected:
    // -- Constructors

    RegularXY(const param::MIRParametrisation&, std::unique_ptr<CustomSpec> projection);

    // -- Methods

    /// Geographic coordinates (lon, lat) of grid coordinates (x, y)
    Point2 lonlat(double x, double y) const;

    // -- Overridden methods

    const eckit::geo::Grid& grid() const override;

    bool sameAs(const Representation&) const override;
    bool includesNorthPole() const override;
    bool includesSouthPole() const override;
    bool isPeriodicWestEast() const override;

    void fillGrib(grib_info&) const override;
    void fillMeshGen(util::MeshGeneratorParameters&) const override;

    void validate(const MIRValuesVector&) const override;
    void makeName(std::ostream&) const override;
    void print(std::ostream&) const override;

    ::atlas::Grid atlasGrid() const override;
    Iterator* iterator() const override;
    size_t numberOfPoints() const override;

private:
    // -- Members

    std::unique_ptr<const eckit::geo::Grid> grid_;
    Point2 false_;
    long edition_;
    bool firstPointBottomLeft_ = false;
    bool uvRelativeToGrid_     = false;  // of the decoded field (GRIB has it in the grid), to re-encode it as is

    // -- Methods

    /// First point (lon, lat) of the scanning, or bottom-left if specified
    Point2 firstPoint() const;
};


}  // namespace mir::repres
