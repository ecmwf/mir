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


#include "mir/data/ParamRepresentation.h"

#include "eckit/exception/Exceptions.h"


namespace mir {
namespace data {


ParamRepresentation::ParamRepresentation(size_t dimension, ParamRepresentation::Component component) :
    dimension_(dimension),
    component_(component) {
    ASSERT(1 <= dimension_ && dimension_ <= 3);
    ASSERT(0 <= component_ && component_ < ALL_COMPONENTS);
}


}  // namespace data
}  // namespace mir
