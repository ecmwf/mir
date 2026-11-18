// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <memory>
#include <vector>

#include "mir/repres/Gridded.h"
#include "mir/util/Types.h"


namespace eckit::geo {
class Grid;
}


namespace mir::repres {


/// Regular grid in projected coordinates (eckit::geo regular_xy): by its gridSpec (eg. of GRIB), or by the keys of a
/// typed grid; the Earth's figure is as encoded (GRIB) or given, otherwise as the gridSpec (typed grids: the default)
class RegularXY : public Gridded {
public:
    // -- Constructors

    /// By the gridSpec, or by the keys of a typed grid and its projection spec (eckit::geo, without the figure)
    explicit RegularXY(const param::MIRParametrisation&, std::unique_ptr<CustomSpec> projection = nullptr);

    // -- Destructor

    ~RegularXY() override;

protected:
    // -- Methods

    bool firstPointBottomLeft() const { return firstPointBottomLeft_; }

    /// Geographic coordinates (lon, lat) of grid coordinates (x, y)
    Point2 lonlat(double x, double y) const;

    /// First point (lon, lat) of the scanning, or bottom-left if specified
    Point2 firstPoint() const;

    /// The encoding edition, as specified or (if not) set to the representation's
    long gribEdition(grib_info&) const;

    /// GRIB encoding as gridType=lambert_azimuthal_equal_area
    void fillGribLambertAzimuthalEqualArea(grib_info&, double standardParallel, double centralLongitude) const;

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
};


}  // namespace mir::repres
