/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Sep 2016


#include "mir/param/ParamInfo.h"

#include "eckit/exception/Exceptions.h"


namespace mir {
namespace param {


ParamInfo::ParamInfo(size_t id, size_t dimension, ParamInfo::Component component) {
    setId(id);
    setDimension(dimension);
    setComponent(component);
}


ParamInfo::ParamInfo(const ParamInfo& other) {
    operator=(other);
}


ParamInfo& ParamInfo::operator=(const ParamInfo& other) {
    id_        = other.id_;
    dimension_ = other.dimension_;
    component_ = other.component_;
    return *this;
}


void ParamInfo::setId(size_t id) {
    id_ = id;
}

void ParamInfo::setDimension(size_t dimension) {
    dimension_ = dimension;
    if (id_ != 0) {
        ASSERT(1 <= dimension_ && dimension_ <= 3);
    }
}

void ParamInfo::setComponent(ParamInfo::Component component) {
    component_ = component;
    if (id_ != 0 && dimension_ > 1) {
        ASSERT(1 <= component_ && component_ < ALL_COMPONENTS);
    }
}


}  // namespace param
}  // namespace mir
