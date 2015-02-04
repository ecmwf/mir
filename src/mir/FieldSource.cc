/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "mir/FieldSource.h"


namespace mir {


FieldSource::FieldSource(const eckit::Params::Ptr& p) : Action(p)
{}


FieldSource::~FieldSource()
{}


atlas::FieldSet::Ptr FieldSource::eval() const
{
  eckit::PathName path ( params()["Input.Path"].as< std::string >() );
  atlas::FieldSet::Ptr fs_inp( new atlas::FieldSet( path ) );

  if( fs_inp->empty() )
    throw eckit::UserError("Input fieldset is empty", Here());

  return fs_inp;
}

ExpPtr field_source(const ExpPtr& e)
{
  NOTIMP;
//	return ExpPtr(  new FieldSource( e ) );
}

} // namespace mir

