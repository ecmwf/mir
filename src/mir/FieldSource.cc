/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "mir/FieldSource.h"

//------------------------------------------------------------------------------------------------------

using namespace eckit;
using namespace atlas;
using namespace atlas::grid;
using namespace mir;

namespace mir {

//------------------------------------------------------------------------------------------------------

FieldSource::FieldSource(const eckit::Params::Ptr& p) : Action(p)
{
}

FieldSource::~FieldSource()
{
}

FieldSet::Ptr FieldSource::eval() const
{
    FieldSet::Ptr fs_inp( new FieldSet( params().get("Input.Path") ) );

    if( fs_inp->empty() )
        throw UserError("Input fieldset is empty", Here());

    return fs_inp;
}

//------------------------------------------------------------------------------------------------------

} // namespace mir
