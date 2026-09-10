// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>
#include <string>
#include <utility>

#include "mir/repres/Gridded.h"


namespace eckit::geo::grid::reduced {
class HEALPix;
}


namespace mir::repres {


class HEALPix final : public Gridded {
public:
    // -- Types

    using points_type = std::pair<std::vector<double>, std::vector<double>>;

    // -- Constructors

    explicit HEALPix(size_t Nside, const std::string& order = "ring");
    explicit HEALPix(const param::MIRParametrisation&);

    // -- Methods

    size_t Nside() const;

    // -- Overridden methods

    const std::string& order() const override;

private:
    // -- Members

    std::unique_ptr<eckit::geo::grid::reduced::HEALPix> grid_;

    mutable points_type points_;

    // -- Methods

    std::string name() const;
    points_type& to_latlons() const;

    // -- Overridden methods

    bool sameAs(const Representation& other) const override;
    void makeName(std::ostream&) const override;

    void fillGrib(grib_info&) const override;
    void fillMeshGen(util::MeshGeneratorParameters&) const override;
    void fillSpec(CustomSpec&) const override;

    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;

    void validate(const MIRValuesVector&) const override;
    size_t numberOfPoints() const override;

    Iterator* iterator() const override;

    bool includesNorthPole() const override { return true; }
    bool includesSouthPole() const override { return true; }
    bool isPeriodicWestEast() const override { return true; }

    std::vector<util::GridBox> gridBoxes() const override;
    ::atlas::Grid atlasGrid() const override;
};


}  // namespace mir::repres
