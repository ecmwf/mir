/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "atlas/grid/Grib.h"

#include "mir/mir_config.h"
#include "mir/FieldSink.h"

//------------------------------------------------------------------------------------------------------

using namespace eckit;
using namespace atlas;
using namespace atlas::grid;
using namespace mir;

namespace mir {

//------------------------------------------------------------------------------------------------------

FieldSink::FieldSink(const eckit::Params::Ptr& p) : Action(p)
{
}

FieldSink::~FieldSink()
{
}

void FieldSink::eval(const FieldSet::Ptr& fs_out) const
{
    //   Timer t("grib output write");

    // Grib::write( *fs_out, path_out ); ///< @todo remove need for clone() with GridSpec's

    Grib::clone( *fs_out,
                      params()["Target.GridPath"],
                      params()["Target.Path"] );
}

//------------------------------------------------------------------------------------------------------

} // namespace mir
