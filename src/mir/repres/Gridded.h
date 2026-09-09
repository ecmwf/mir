// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"


namespace eckit::geo::projection {
class Rotation;
}


namespace mir::repres {


class Gridded : public Representation {
public:
    // -- Types

    using Rotation = eckit::geo::projection::Rotation;

    // -- Exceptions
    // None

    // -- Constructors

    Gridded(const param::MIRParametrisation&);
    Gridded(const util::BoundingBox&);

    // -- Destructor

    ~Gridded() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Constructors

    Gridded();

    // -- Members

    util::BoundingBox bbox_;

    // -- Methods
    // None

    // -- Overridden methods

    void fillSpec(CustomSpec&) const override;
    void reorder(MIRValuesVector&) const override;

    util::Domain domain() const override;
    const util::BoundingBox& boundingBox() const override;
    bool getLongestElementDiagonal(double&) const override;

    std::string intersectionOnCrop() const override { return "none"; }
    bool crop(util::BoundingBox&, util::IndexMapping&) const override;

    // -- Class members
    // None

    // -- Class methods

    static atlas::Grid rotate_atlas_grid(const Rotation& rotation, const atlas::Grid& grid);

private:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    size_t numberOfValues() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres
