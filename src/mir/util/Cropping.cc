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


#include "mir/util/Cropping.h"


namespace mir {
namespace util {


Cropping::Cropping() :
    active_(false) {
}


Cropping::~Cropping() {
}


void Cropping::boundingBox(const BoundingBox& bbox) {
    bbox_ = bbox;
    active_ = true;
}


bool Cropping::operator==(const Cropping& other) const {
    return (active_ == other.active_) && (bbox_ == other.bbox_);
}


const BoundingBox& Cropping::boundingBox() const {
    ASSERT(active_);
    return bbox_;
}


void Cropping::print(std::ostream &out) const {
    if (active_) {
        out << bbox_;
    } else {
        out << "none";
    }
}


bool Cropping::active() const {
    return active_;
}


}  // namespace util
}  // namespace mir

