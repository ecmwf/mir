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

#include "eckit/memory/NonCopyable.h"
#include "eckit/value/Value.h"
#include "eckit/value/Properties.h"

//------------------------------------------------------------------------------------------------------

namespace mir {

//------------------------------------------------------------------------------------------------------

class Params : public eckit::NonCopyable {

public: // types

    typedef std::shared_ptr<Params> Ptr;
    typedef std::list< Params::Ptr > List;
    typedef std::string  key_t;
    typedef eckit::Value value_t;

public: // methods

    virtual bool exists( const key_t& key ) const = 0;

    virtual value_t get( const key_t& key ) const = 0;

};

//------------------------------------------------------------------------------------------------------

class CompositeParams : public Params {

public: // methods

    CompositeParams( const Params::List& plist );

    virtual bool exists( const key_t& key ) const;

    virtual value_t get( const key_t& key ) const;

private: // members

    Params::List params_;

};

//------------------------------------------------------------------------------------------------------

class UserParams : public Params {

public: // methods

    UserParams();

    virtual bool exists( const key_t& key ) const;

    virtual value_t get( const key_t& key ) const;

};

//------------------------------------------------------------------------------------------------------

class ValueParams : public Params {
public: // methods

    ValueParams() : props_() {}
    ValueParams( const eckit::Properties& p ) : props_(p) {}

    virtual bool exists( const key_t& key ) const;

    virtual value_t get( const key_t& key ) const;

protected: // members

    eckit::Properties props_;

};

//-------------------------------------------------------------------------------------------

template < class Derived >
class DispatchParams : public Params {

public: // methods

    DispatchParams() {}

public: // methods

    virtual bool exists( const key_t& key ) const
    {
        return ( parametrizations_.find(key) != parametrizations_.end() );
    }

    virtual value_t get( const key_t& key ) const
    {
        typename store_t::const_iterator i = parametrizations_.find(key);
        if( i != parametrizations_.end() )
        {
            parametrizer_t fptr = i->second;
            const Derived* pobj = static_cast<const Derived*>(this);
            return (pobj->*fptr)( key );
        }
        else
            return Params::value_t();
    }

protected: // members

    typedef Params::value_t ( Derived::* parametrizer_t ) ( const key_t& ) const ;
    typedef std::map< std::string, parametrizer_t > store_t;

    store_t parametrizations_;

};

//-------------------------------------------------------------------------------------------

class MarsParams : public DispatchParams<MarsParams> {
public:

    /// this will be build with a request

    value_t getUser( const key_t& ) const { return "rdx"; }

    MarsParams()
    {
        parametrizations_["mars.user"] = &MarsParams::getUser;
    }
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

//------------------------------------------------------------------------------------------------------

} // namespace mir

#endif
