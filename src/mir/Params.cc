/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/filesystem/PathName.h"
#include "eckit/config/Resource.h"

#include "mir/Params.h"

//------------------------------------------------------------------------------------------------------

using namespace eckit;

namespace mir {

//------------------------------------------------------------------------------------------------------

UserParams::UserParams()
{
    dispatch_["MaskPath"] = &UserParams::getMask;
}

Params::value_t UserParams::getMask(const Params::key_t &) const
{
    std::string mpath = Resource<std::string>("-mask",""); // mask may be empty

    if( !mpath.empty() )
        return value_t( mpath );
    else
        return value_t();
}

//------------------------------------------------------------------------------------------------------

MirContext::MirContext( Params** r )
{
    Params::Ptr runtime( new RuntimeParams(r) );
    Params::Ptr input( new ScopeParams( "Input", runtime ) );

    push_back( input );

    Params::Ptr ecmwf( new ProfileParams() );

    push_back( ecmwf );
}

//------------------------------------------------------------------------------------------------------

} // namespace mir
