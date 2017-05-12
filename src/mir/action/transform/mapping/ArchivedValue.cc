/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date May 2017


#include "mir/action/transform/mapping/ArchivedValue.h"

#include "eckit/exception/Exceptions.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {
namespace transform {
namespace mapping {


namespace {
static MappingBuilder< ArchivedValue > __mapping1("av");
static MappingBuilder< ArchivedValue > __mapping2("archived value");
}


ArchivedValue::ArchivedValue(const param::MIRParametrisation& parametrisation) : Mapping(parametrisation) {
    ASSERT(parametrisation.get("truncation", truncation_));
    ASSERT(truncation_);
}


ArchivedValue::~ArchivedValue() {}


size_t ArchivedValue::getTruncationFromPointsPerLatitude(const size_t&) const {
    return truncation_;
}


size_t ArchivedValue::getPointsPerLatitudeFromTruncation(const size_t& T) const {
    NOTIMP;
}


void ArchivedValue::print(std::ostream& out) const {
    out << "ArchivedValue[truncation=" << truncation_ << "]";
}


}  // namespace mapping
}  // namespace transform
}  // namespace action
}  // namespace mir
