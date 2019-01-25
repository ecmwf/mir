/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date May 2015

#include "mir/method/Cropping.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/MD5.h"

namespace mir {
namespace method {

Cropping::Cropping() : active_(false) {}

Cropping::~Cropping() = default;

void Cropping::hash(eckit::MD5& md5) const {
    md5.add(bbox_);
    md5.add(active_);
}

void Cropping::boundingBox(const util::BoundingBox& bbox) {
    bbox_ = bbox;
    active_ = true;
}

bool Cropping::operator==(const Cropping& other) const {
    return (active_ == other.active_) && (bbox_ == other.bbox_);
}

const util::BoundingBox& Cropping::boundingBox() const {
    ASSERT(active_);
    return bbox_;
}

void Cropping::print(std::ostream& out) const {
    if (active_) {
        out << bbox_;
    } else {
        out << "none";
    }
}

} // namespace method
} // namespace mir
