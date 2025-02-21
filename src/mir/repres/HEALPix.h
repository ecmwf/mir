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

#include <string>

#include "eckit/geo/grid/HEALPix.h"

#include "mir/repres/Gridded.h"


namespace mir::repres {


class HEALPix : public Gridded {
public:
    // -- Types

    using Ordering = eckit::geo::Ordering;

    // -- Constructors

    explicit HEALPix(size_t Nside, Ordering ordering = Ordering::healpix_ring);
    explicit HEALPix(const param::MIRParametrisation&);

    HEALPix(const HEALPix&) = delete;
    HEALPix(HEALPix&&)      = delete;

    // -- Destructor

    ~HEALPix() override = default;

    // -- Operators

    HEALPix& operator=(const HEALPix&) = delete;
    HEALPix& operator=(HEALPix&&)      = delete;

private:
    // -- Members

    eckit::geo::grid::HEALPix grid_;

    // -- Overridden methods

    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;
    void makeName(std::ostream&) const override;

    void fillGrib(grib_info&) const override;
    void fillJob(api::MIRJob&) const override;
    void fillMeshGen(util::MeshGeneratorParameters&) const override;

    bool sameAs(const Representation&) const override;
    void validate(const MIRValuesVector& values) const override;

    ::atlas::Grid atlasGrid() const override;
    std::vector<util::GridBox> gridBoxes() const override;

    size_t numberOfPoints() const override;

    bool includesNorthPole() const override { return true; }
    bool includesSouthPole() const override { return true; }
    bool isPeriodicWestEast() const override { return true; }

    Iterator* iterator() const override;
};


}  // namespace mir::repres
