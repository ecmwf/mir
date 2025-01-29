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


#include "mir/iterator/UnstructuredIterator.h"

#include "mir/util/Exceptions.h"


namespace mir::iterator {


UnstructuredIterator::UnstructuredIterator(const std::vector<double>& latitudes,
                                           const std::vector<double>& longitudes) :
    count_(0), size_(latitudes.size()), latitudes_(latitudes), longitudes_(longitudes), first_(true) {
    ASSERT(latitudes_.size() == longitudes_.size());
}


void UnstructuredIterator::print(std::ostream& out) const {
    out << "UnstructuredGridIterator[";
    Iterator::print(out);
    out << "]";
}


bool UnstructuredIterator::next(Latitude& lat, Longitude& lon) {
    if ((first_ ? count_ : ++count_) < size_) {
        first_ = false;
        lat    = latitudes_[count_];
        lon    = longitudes_[count_];

        return true;
    }

    return false;
}


}  // namespace mir::iterator
