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

    UnstructuredIterator(const UnstructuredIterator&) = delete;
    UnstructuredIterator(UnstructuredIterator&&)      = delete;

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators

    void operator=(const UnstructuredIterator&) = delete;
    void operator=(UnstructuredIterator&&)      = delete;

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
