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


#include "mir/action/transform/mapping/TMapping.h"


namespace mir {
namespace action {
namespace transform {
namespace mapping {


namespace {
static MappingBuilder< TMapping<1> > __mapping1("linear");
static MappingBuilder< TMapping<2> > __mapping2("quadratic");
static MappingBuilder< TMapping<3> > __mapping3("cubic");
static MappingBuilder< TMapping<4> > __mapping4("quartic");
}


}  // namespace mapping
}  // namespace transform
}  // namespace action
}  // namespace mir

