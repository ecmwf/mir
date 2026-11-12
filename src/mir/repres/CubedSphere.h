// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "mir/repres/Gridded.h"


namespace eckit::geo::grid {
class CubedSphere;
}


namespace mir::repres {


/**
 * @brief Cubed sphere (eckit::geo, of any panels convention, e.g. LFRic), by its faces, nodes or edges arrangement
 * @details Its own representation (not as unstructured points) so that cubed sphere specifics have a place: the
 * mesh generator, and the GRIB encoding.
 */
class CubedSphere final : public Gridded {
public:
    // -- Types

    using points_type = std::pair<std::vector<double>, std::vector<double>>;

    // -- Constructors

    explicit CubedSphere(const param::MIRParametrisation&);

    // -- Destructor

    ~CubedSphere() override;

private:
    // -- Constructors

    explicit CubedSphere(const eckit::geo::grid::CubedSphere*);

    // -- Members

    std::unique_ptr<const eckit::geo::grid::CubedSphere> grid_;

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

    bool includesNorthPole() const override { return true; }
    bool includesSouthPole() const override { return true; }
    bool isPeriodicWestEast() const override { return true; }

    ::atlas::Grid atlasGrid() const override;
};


}  // namespace mir::repres
