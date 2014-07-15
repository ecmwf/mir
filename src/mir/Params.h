/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @date July 2014

#ifndef mir_Params_H
#define mir_Params_H

#include "eckit/memory/Owned.h"
#include "eckit/memory/SharedPtr.h"
#include "eckit/value/Value.h"
#include "eckit/value/Properties.h"

//------------------------------------------------------------------------------------------------------

namespace mir {

//------------------------------------------------------------------------------------------------------

class Params : public eckit::Owned {

public: // types

    typedef eckit::SharedPtr<Params> Ptr;

    typedef std::list< Params::Ptr > List;

    typedef std::string  key_t;
    typedef eckit::Value value_t;

public: // methods

    virtual ~Params();

    Ptr self() { return Params::Ptr(this); }

    virtual value_t get( const key_t& key,  Params* r = NULL ) const = 0;

};

//------------------------------------------------------------------------------------------------------

class CompositeParams : public Params {

public: // methods

    CompositeParams();
    CompositeParams( const Params::List& );

    virtual value_t get( const key_t& key,  Params* r = NULL ) const;

    void push_front( const Params::Ptr& p );
    void push_back( const Params::Ptr& p );

private: // members

    Params::List plist_;

};

//------------------------------------------------------------------------------------------------------

class UserParams : public Params {

public: // methods

    UserParams();

    virtual value_t get( const key_t& key,  Params* r = NULL ) const;

};

//------------------------------------------------------------------------------------------------------

class ValueParams : public Params {

public: // methods

    ValueParams() : props_() {}
    ValueParams( const eckit::Properties& p ) : props_(p) {}

    virtual value_t get( const key_t& key,  Params* r = NULL ) const;

    void set( const key_t& k, const value_t& v );

protected: // members

    eckit::Properties props_;

};

//-------------------------------------------------------------------------------------------

template < class Derived >
class DispatchParams : public Params {

public: // methods

    DispatchParams() {}

public: // methods

    virtual value_t get( const key_t& key,  Params* r = NULL ) const
    {
        typename store_t::const_iterator i = dispatch_.find(key);
        if( i != dispatch_.end() )
        {
            parametrizer_t fptr = i->second;
            const Derived* pobj = static_cast<const Derived*>(this);
            return (pobj->*fptr)( key, r );
        }
        else
            return Params::value_t();
    }

protected: // members

    typedef Params::value_t ( Derived::* parametrizer_t ) ( const key_t&, Params* ) const ;
    typedef std::map< std::string, parametrizer_t > store_t;

    store_t dispatch_;

};

//-------------------------------------------------------------------------------------------

class ScopedParams : public Params {

public: // methods

    ScopedParams( const key_t& scope_key, const Params::Ptr& p );

    virtual value_t get( const key_t& key,  Params* r = NULL ) const;

private: // members

    key_t scope_;
    Params::Ptr p_;

};

//-------------------------------------------------------------------------------------------

class RuntimeParams : public Params {

public: // methods

    virtual value_t get( const key_t& key,  Params* r = NULL ) const;

};

//-------------------------------------------------------------------------------------------

class ECMWFParams : public ValueParams {
public:

    ECMWFParams()
    {
        props_.set( "resol", "auto" );
        props_.set( "grid", "1/1" );
        props_.set( "lsm", "hres.grib" );
    }
};

//-------------------------------------------------------------------------------------------

class MirContext : public CompositeParams {
public:

    MirContext();

};

//------------------------------------------------------------------------------------------------------

} // namespace mir

#endif
