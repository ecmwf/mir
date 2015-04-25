/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "atlas/io/Grib.h"
#include "atlas/io/PointCloud.h"

#include "mir/mir_config.h"
#include "mir/FieldSink.h"

//------------------------------------------------------------------------------------------------------

using namespace eckit;
using namespace atlas;
using namespace atlas::io;
using namespace mir;

namespace mir {

//------------------------------------------------------------------------------------------------------

FieldSink::FieldSink(const Params & p) : Action(p)
{
}

FieldSink::~FieldSink()
{
}

void FieldSink::eval(const FieldSet::Ptr& fs_out) const
{
    //   Timer t("grib output write");

    // Grib::write( *fs_out, path_out ); ///< @todo remove need for clone() with GridSpec's

  std::string oformat( params()["Target.OutputFormat"] );

  if( oformat == "grib" )
  {
    Grib::clone( *fs_out,
                      params()["Target.GridPath"],
                      params()["Target.Path"] );
    return;
  }

  if( oformat == "pointcloud" )
  {
    PointCloud::write( params()["Target.Path"], *fs_out );
    return;
  }

  throw UserError( std::string("unknown output format ") + oformat, Here() );

}

ExpPtr field_sink(const ExpPtr& e)
{
  NOTIMP;
//	return ExpPtr( new FieldSink( e ) );
}

//------------------------------------------------------------------------------------------------------

} // namespace mir
