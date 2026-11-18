// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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


bool UnstructuredIterator::next(PointLonLat& point) {
    if ((first_ ? count_ : ++count_) < size_) {
        first_ = false;
        point  = PointLonLat{longitudes_[count_], latitudes_[count_]};

        return true;
    }

    return false;
}


}  // namespace mir::iterator
