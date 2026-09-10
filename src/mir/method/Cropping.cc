// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
