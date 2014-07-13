/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "mir/Params.h"

//------------------------------------------------------------------------------------------------------

using namespace eckit;

namespace mir {

//------------------------------------------------------------------------------------------------------

CompositeParams::CompositeParams(const Params::List &plist) : params_(plist) {}

bool CompositeParams::exists( const key_t& key ) const
{
    for( Params::List::const_iterator citr = params_.begin(); citr != params_.end(); ++citr )
        if( (*citr)->exists(key) )	return true;
    return false;
}

CompositeParams::value_t CompositeParams::get( const key_t& key ) const
{
    for( Params::List::const_iterator citr = params_.begin(); citr != params_.end(); ++citr )
    {
        if( (*citr)->exists(key) )
            return (*citr)->get(key);
    }
    return Value();
}

//------------------------------------------------------------------------------------------------------

bool ValueParams::exists(const key_t& key) const
{
    return props_.has(key);
}

Params::value_t ValueParams::get(const key_t& key) const
{
    return props_.get(key);
}

//------------------------------------------------------------------------------------------------------

UserParams::UserParams()
{
}

bool UserParams::exists(const Params::key_t &key) const
{
}

Params::value_t UserParams::get(const Params::key_t &key) const
{
}

//------------------------------------------------------------------------------------------------------

} // namespace mir
