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


#include "mir/method/Cropping.h"

#include "eckit/utils/MD5.h"

#include "mir/util/Exceptions.h"


namespace mir::method {


Cropping::Cropping() : active_(false) {}


void Cropping::hash(eckit::MD5& md5) const {
    md5.add(bbox_);
    md5.add(active_);
}


bool Cropping::sameAs(const Cropping& other) const {
    return active_ && other.active_ ? bbox_ == other.bbox_ : active_ == other.active_;
}


void Cropping::boundingBox(const util::BoundingBox& bbox) {
    bbox_   = bbox;
    active_ = true;
}


const util::BoundingBox& Cropping::boundingBox() const {
    ASSERT(active_);
    return bbox_;
}


void Cropping::print(std::ostream& out) const {
    if (active_) {
        out << bbox_;
    }
    else {
        out << "none";
    }
}


}  // namespace mir::method
