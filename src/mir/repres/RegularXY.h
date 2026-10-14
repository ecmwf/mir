// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "mir/repres/Gridded.h"


namespace eckit::geo {
class Grid;
}


namespace mir::repres {


/// Regular grid in projected coordinates (eg. swisslv95), points in geographic coordinates
class RegularXY final : public Gridded {
public:
    // -- Types

    using points_type = std::pair<std::vector<double>, std::vector<double>>;

    // -- Constructors

    explicit RegularXY(const param::MIRParametrisation&);

    // -- Destructor

    ~RegularXY() override;

private:
    // -- Constructors

    explicit RegularXY(const eckit::geo::Grid*);

    // -- Members

    std::unique_ptr<const eckit::geo::Grid> grid_;

    mutable points_type points_;

    // -- Methods

    const points_type& to_latlons() const;

    // -- Overridden methods

    bool sameAs(const Representation&) const override;
    void makeName(std::ostream&) const override;

    void fillGrib(grib_info&) const override;
    void fillMeshGen(util::MeshGeneratorParameters&) const override;
    void fillSpec(CustomSpec&) const override;

    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;

    void validate(const MIRValuesVector&) const override;
    size_t numberOfPoints() const override;

    Iterator* iterator() const override;

    bool includesNorthPole() const override;
    bool includesSouthPole() const override;
    bool isPeriodicWestEast() const override;

    ::atlas::Grid atlasGrid() const override;
};


}  // namespace mir::repres
