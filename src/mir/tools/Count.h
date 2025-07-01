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

    using Distance = std::pair<double, double>;

    // -- Members

    const util::BoundingBox bbox_;
    bool first_;

    size_t count_;
    size_t countTotal_;

    double n_;
    double s_;
    double e_;
    double w_;

    std::set<Distance> nn_;
    std::set<Distance> ss_;
    std::set<Distance> ww_;
    std::set<Distance> ee_;

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
