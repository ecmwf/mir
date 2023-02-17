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

#include "mir/repres/Iterator.h"
#include "mir/util/Exceptions.h"


namespace mir::iterator {


class UnstructuredIterator : public repres::Iterator {
public:
    // -- Exceptions
    // None

    // -- Constructors
    UnstructuredIterator(const std::vector<double>& latitudes, const std::vector<double>& longitudes) :
        count_(0), size_(latitudes.size()), latitudes_(latitudes), longitudes_(longitudes), first_(true) {
        ASSERT(latitudes_.size() == longitudes_.size());
    }

    UnstructuredIterator(const UnstructuredIterator&) = delete;

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators

    UnstructuredIterator& operator=(const UnstructuredIterator&) = delete;

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

    // From Iterator

    void print(std::ostream& out) const override {
        out << "UnstructuredGridIterator[";
        Iterator::print(out);
        out << "]";
    }

    bool next(Latitude& lat, Longitude& lon) override {
        if ((first_ ? count_ : ++count_) < size_) {
            first_ = false;
            lat    = latitudes_[count_];
            lon    = longitudes_[count_];

            return true;
        }
        return false;
    }

    size_t index() const override { return count_; }

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::iterator
