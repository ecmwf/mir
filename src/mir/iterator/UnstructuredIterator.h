// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <vector>

#include "mir/repres/Iterator.h"
#include "mir/util/Types.h"


namespace mir::iterator {


class UnstructuredIterator : public repres::Iterator {
public:
    // -- Exceptions
    // None

    // -- Constructors

    UnstructuredIterator(const std::vector<double>& latitudes, const std::vector<double>& longitudes);

    // -- Destructor
    // None

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

private:
    // -- Members

    size_t count_;
    const size_t size_;
    const std::vector<double>& latitudes_;
    const std::vector<double>& longitudes_;
    bool first_;

    // -- Methods
    // None

    // -- Overridden methods

    void print(std::ostream& out) const override;
    bool next(Latitude& lat, Longitude& lon) override;
    size_t index() const override { return count_; }

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::iterator
