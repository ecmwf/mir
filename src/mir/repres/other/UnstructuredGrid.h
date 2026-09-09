// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <vector>

#include "mir/repres/Gridded.h"


namespace eckit {
class PathName;
}


namespace mir::repres::other {


class UnstructuredGrid : public Gridded {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit UnstructuredGrid(const eckit::PathName&);
    explicit UnstructuredGrid(const param::MIRParametrisation&);
    UnstructuredGrid(const std::vector<double>& latitudes, const std::vector<double>& longitudes,
                     const util::BoundingBox& = util::BoundingBox());

    // -- Destructor

    ~UnstructuredGrid() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    static void save(const eckit::PathName&, const std::vector<double>& latitudes,
                     const std::vector<double>& longitudes, bool binary);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    void print(std::ostream&) const override;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::vector<double> latitudes_;
    std::vector<double> longitudes_;

    // -- Methods
    // None

    // -- Overridden methods

    void fillGrib(grib_info&) const override;
    void fillSpec(CustomSpec&) const override;
    void fillMeshGen(util::MeshGeneratorParameters&) const override;

    atlas::Grid atlasGrid() const override;
    void validate(const MIRValuesVector&) const override;

    util::Domain domain() const override;
    Iterator* iterator() const override;
    void makeName(std::ostream&) const override;
    bool sameAs(const Representation&) const override;

    // Domain operations
    bool isPeriodicWestEast() const override;
    bool includesNorthPole() const override;
    bool includesSouthPole() const override;

    size_t numberOfPoints() const override;
    const Gridded* croppedRepresentation(const util::BoundingBox&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres::other
