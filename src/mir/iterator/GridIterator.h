// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>

#include "eckit/geo/Grid.h"

#include "mir/repres/Iterator.h"


namespace mir::iterator {


/// Points of a grid (eckit::geo) by its own iterator, in its order or the canonical (its default); of a rotated grid,
/// the points are in the grid frame (unrotated) and the rotation is the iterator's (pointRotated)
class GridIterator final : public repres::Iterator {
public:
    // -- Constructors

    explicit GridIterator(const eckit::geo::Grid&, bool canonical = false);

private:
    // -- Members

    std::unique_ptr<const eckit::geo::Grid> grid_;
    eckit::geo::Grid::Iterator it_;
    bool first_ = true;

    // -- Overridden methods

    void print(std::ostream&) const override;
    bool next(Latitude&, Longitude&) override;
    size_t index() const override { return it_.index(); }
};


}  // namespace mir::iterator
