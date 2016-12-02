/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/util/OffsetIterator.h"
#include "eckit/types/FloatCompare.h"


namespace mir {
namespace util {


OffsetIterator::OffsetIterator(Iterator* iterator, double northwards, double eastwards) :
    iterator_(iterator),
    northwards_(northwards),
    eastwards_(eastwards) {
}


OffsetIterator::~OffsetIterator() {
}


void OffsetIterator::print(std::ostream& out) const {
    out << "OffsetIterator[iterator=" << *iterator_
        << ",northwards=" << northwards_
        << ",eastwards=" << eastwards_ << "]";
}


bool OffsetIterator::next(double& lat, double& lon) {

    typedef eckit::FloatCompare<double> cmp;

    while (iterator_->next(lat, lon)) {

        lon += eastwards_;
        lat += northwards_;

        if (cmp::isStrictlyGreater(lat, 90) || cmp::isStrictlyGreater(-90, lat)) {
            continue;
        }


        return true;
    }
    return false;
}


}  // namespace repres
}  // namespace mir

