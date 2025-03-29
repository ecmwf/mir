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

#include <utility>
#include <vector>

#include "eckit/geo/grid/unstructured/UnstructuredGeneric.h"

#include "mir/repres/Gridded.h"


namespace eckit {
class PathName;
}


namespace mir::repres::other {


class UnstructuredGrid : public Gridded {
public:
    // -- Constructors

    explicit UnstructuredGrid(const eckit::PathName&);
    explicit UnstructuredGrid(const param::MIRParametrisation&);
    UnstructuredGrid(const std::vector<double>& latitudes, const std::vector<double>& longitudes,
                     const util::BoundingBox& = util::BoundingBox(), bool checkDuplicatePoints = true);

    UnstructuredGrid(const UnstructuredGrid&) = delete;
    UnstructuredGrid(UnstructuredGrid&&)      = delete;

    // -- Destructor

    ~UnstructuredGrid() override;

    // -- Operators

    void operator=(const UnstructuredGrid&)  = delete;
    void operator=(const UnstructuredGrid&&) = delete;

    // -- Methods

    static void save(const eckit::PathName&, const std::vector<double>& latitudes,
                     const std::vector<double>& longitudes, bool binary);

private:
    // -- Types

    using latlon_t = std::pair<std::vector<double>, std::vector<double>>;

    // -- Constructors

    UnstructuredGrid(latlon_t&&, const util::BoundingBox&, bool checkDuplicatePoints);

    // -- Members

    eckit::geo::grid::unstructured::UnstructuredGeneric grid_;

    std::vector<double> latitudes_;
    std::vector<double> longitudes_;

    // -- Overridden methods

    bool extendBoundingBoxOnIntersect() const override;
    void print(std::ostream&) const override;

    void fillGrib(grib_info&) const override;
    void fillJob(api::MIRJob&) const override;
    void fillMeshGen(util::MeshGeneratorParameters&) const override;

    atlas::Grid atlasGrid() const override;
    void validate(const MIRValuesVector&) const override;

    util::Domain domain() const override;
    Iterator* iterator() const override;
    void makeName(std::ostream&) const override;
    bool sameAs(const Representation&) const override;

    bool isPeriodicWestEast() const override;
    bool includesNorthPole() const override;
    bool includesSouthPole() const override;

    size_t numberOfPoints() const override;
    const Gridded* croppedRepresentation(const util::BoundingBox&) const override;
};


}  // namespace mir::repres::other
