// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <set>
#include <vector>

#include "mir/util/BoundingBox.h"
#include "mir/util/Types.h"


namespace eckit {
class JSON;
}

namespace mir::repres {
class Representation;
}  // namespace mir::repres


namespace mir::tools {


class Count {
public:
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    Count(const util::BoundingBox&);
    Count(std::vector<double>& area);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void reset();

    void print(std::ostream&) const;
    void json(eckit::JSON&, bool enclose = true) const;

    void countOnNamedGrid(const std::string& grid);
    void countOnGridIncrements(const std::vector<double>& grid);
    void countOnRepresentation(const repres::Representation&);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Types

    using DistanceLat = std::pair<double, double>;
    using DistanceLon = std::pair<double, double>;

    // -- Members

    const util::BoundingBox bbox_;
    bool first_;

    size_t count_;
    size_t countTotal_;

    double n_;
    double s_;
    double e_;
    double w_;

    std::set<DistanceLat> nn_;
    std::set<DistanceLat> ss_;
    std::set<DistanceLon> ww_;
    std::set<DistanceLon> ee_;

    // -- Methods

    void count(const PointLonLat&);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::tools
