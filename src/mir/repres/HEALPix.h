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

#include "eckit/geo/grid/HEALPix.h"

#include "mir/repres/Geo.h"


namespace mir::repres {


class HEALPix : public Geo {
public:
    // -- Types

    using Ordering = eckit::geo::Ordering;

    // -- Constructors

    explicit HEALPix(size_t Nside, Ordering ordering = Ordering::healpix_ring);
    explicit HEALPix(const param::MIRParametrisation&);

private:
    // -- Members

    const eckit::geo::grid::HEALPix& grid_;

    // -- Overridden methods

    void makeName(std::ostream&) const override;

    void fillGrib(grib_info&) const override;
    void fillMeshGen(util::MeshGeneratorParameters&) const override;

    ::atlas::Grid atlasGrid() const override;
    std::vector<util::GridBox> gridBoxes() const override;

    bool includesNorthPole() const override { return true; }
    bool includesSouthPole() const override { return true; }
    bool isPeriodicWestEast() const override { return true; }
};


}  // namespace mir::repres
