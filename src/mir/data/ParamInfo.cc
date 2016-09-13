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


#include "mir/data/ParamInfo.h"

#include "eckit/exception/Exceptions.h"


namespace mir {
namespace data {


ParamInfo::ParamInfo(size_t id, size_t dimension, ParamInfo::Component component) {
    set(id, dimension, component);
}


ParamInfo::ParamInfo(const ParamInfo& other) {
    operator=(other);
}


ParamInfo& ParamInfo::operator=(const ParamInfo& other) {
    set(other.id_, other.dimension_, other.component_);
    return *this;
}


void ParamInfo::set(size_t id, size_t dimension, Component component) {
    id_        = id;
    dimension_ = dimension;
    component_ = component;

    if (id_ != 0) {
        ASSERT(1 <= dimension_ && dimension_ <= 3);
        ASSERT((dimension_ == 1) || (1 <= component_ && component_ < ALL_COMPONENTS));
    }
}


}  // namespace data
}  // namespace mir
