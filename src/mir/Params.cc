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

Params::~Params()
{

}

//------------------------------------------------------------------------------------------------------

CompositeParams::CompositeParams() : plist_() {}

CompositeParams::CompositeParams(const Params::List& plist) : plist_(plist) {}

CompositeParams::value_t CompositeParams::get( const key_t& key,  Params* r ) const
{
    for( Params::List::const_iterator citr = plist_.begin(); citr != plist_.end(); ++citr )
    {
        Value v = (*citr)->get(key,r);
        if( !v.isNil() )
            return v;
    }
    return Value();
}

void CompositeParams::push_front(const Params::Ptr& p)
{
    plist_.push_front(p);
}

void CompositeParams::push_back(const Params::Ptr& p)
{
    plist_.push_back(p);
}

//------------------------------------------------------------------------------------------------------

Params::value_t ValueParams::get(const key_t& key, Params* r) const
{
    return props_.get(key); // returns Value Nil if doesn't exist
}

void ValueParams::set(const Params::key_t& k, const Params::value_t& v)
{
    props_.set(k,v);
}

//------------------------------------------------------------------------------------------------------

UserParams::UserParams()
{
}


Params::value_t UserParams::get(const Params::key_t& key, Params* r) const
{
    NOTIMP;
}

//------------------------------------------------------------------------------------------------------

Params::value_t RuntimeParams::get(const Params::key_t& key, Params* r) const
{
    if( r )
        return r->get(key,r);
    else
        return value_t();
}

//------------------------------------------------------------------------------------------------------

MirContext::MirContext()
{
    Params::Ptr runtime( new RuntimeParams() );
    Params::Ptr input( new ScopedParams( "Input", runtime ) );

    push_back( input );

    Params::Ptr ecmwf( new ECMWFParams() );

    push_back( ecmwf );
}

//------------------------------------------------------------------------------------------------------

ScopedParams::ScopedParams(const Params::key_t& scope_key, const Params::Ptr& p ) :
    scope_(scope_key),
    p_(p)
{
    ASSERT(p_);
}

Params::value_t ScopedParams::get(const Params::key_t& key, Params *r) const
{
    return p_->get( scope_ + "." + key , r);
}

//------------------------------------------------------------------------------------------------------

} // namespace mir
