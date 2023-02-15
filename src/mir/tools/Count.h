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

    using DistanceLat = std::pair<Latitude, Latitude>;
    using DistanceLon = std::pair<Longitude, Longitude>;

    // -- Members

    const util::BoundingBox bbox_;
    bool first_;

    size_t count_;
    size_t countTotal_;

    Latitude n_;
    Latitude s_;
    Longitude e_;
    Longitude w_;

    std::set<DistanceLat> nn_;
    std::set<DistanceLat> ss_;
    std::set<DistanceLon> ww_;
    std::set<DistanceLon> ee_;

    // -- Methods

    void count(const PointLatLon&);

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
