/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <string>

#include "eckit/memory/NonCopyable.h"
#include "eckit/value/Properties.h"
#include "eckit/io/Buffer.h"
#include "eckit/log/CallbackChannel.h"
#include "eckit/runtime/LibBehavior.h"
#include "eckit/runtime/Context.h"

#include "atlas/grid/FieldSet.h"
#include "atlas/grid/Grib.h"

#include "mir/mars_api.h"

#include "mir/Interpolate.h"
#include "mir/Params.h"
#include "mir/FieldContext.h"

using namespace eckit;
using namespace eckit::grib;
using namespace atlas;
using namespace atlas::grid;
using namespace mir;

//------------------------------------------------------------------------------------------------------

class MarsParams : public DispatchParams<MarsParams> {

public: // methods

    MarsParams( const Params& p) : values_(p)
    {
//        dispatch_["Target.Grid"] = &MarsParams::getGrid; // examples
//        dispatch_["Target.Area"] = &MarsParams::getArea;
    }

//    /// this will be build with a request
//    value_t getGrid( const key_t& ) const
//    {
//        /// translate from GRID=1/1 to ValueMap( GridType = "regular_ll", LatInc = 1, LonInc = 1 )
//    }

    virtual value_t get( const key_t& k ) const
    {
        value_t v = values_.get(k);
        if( !v.isNil() )
            return v;
        else
            return DispatchParams<MarsParams>::get(k);
    }

private: // members

    const Params& values_;
};

//------------------------------------------------------------------------------------------------------

class MarsContext : public MirContext {

public: // methods

    MarsContext() :
        MirContext( &runtime_ ),
        frozen_(false),
        buffer_( Resource<size_t>( "MirFieldBufferSize;$MIR_FIELD_BUFFER_SIZE", 60*1024*1024) )
    {
        Params::Ptr mars( new MarsParams( mars_values_ ) );

        push_front( mars->self() );
    }

    virtual ~MarsContext() { std::cout << "Destroying MarsContext" << std::endl; }

    bool frozen() const { return frozen_; }
    void freeze() { frozen_ = true; }

    ValueParams& mars_values() { return mars_values_; }

    void set_runtime( Params::Ptr p ) { runtime_ = p.get(); }

    Buffer& buffer() { return buffer_; }

private: // members

    bool frozen_;

    ValueParams mars_values_;
    Params* runtime_;

    Buffer buffer_;
};

//------------------------------------------------------------------------------------------------------
//

extern "C" {

mir_err mir_create_context( mir_context_ptr* ctxt )
{
    LibBehavior* b = new LibBehavior();

    Context::instance().behavior( b );

    MarsContext* mctxt = new MarsContext();
    ASSERT( mctxt );
    *ctxt = reinterpret_cast<mir_context_ptr>(mctxt);

    return MIR_SUCCESS;
}

mir_err mir_set_context_logger(mir_context_ptr ctxt, logger_proc logger )
{
    dynamic_cast<CallbackChannel&>(Log::info()).register_callback( logger, &ctxt );

    return MIR_SUCCESS;
}

mir_err mir_set_context_value(mir_context_ptr ctxt, const char* key, const char* value)
{
    if(!ctxt) return MIR_INVALID_CONTEXT;

    MarsContext* mctxt = reinterpret_cast<MarsContext*>(ctxt);

    if( mctxt->frozen() )
        return MIR_CONTEXT_FROZEN;

    mctxt->mars_values().set( key, eckit::Value(value) );

    return MIR_SUCCESS;
}

mir_err mir_interpolate(mir_context_ptr ctxt, const void* buffin, size_t sin, void **buffout, size_t* sout)
{
    if(!ctxt) return MIR_INVALID_CONTEXT;

    ASSERT( buffout );
    ASSERT( *buffout == NULL );
    ASSERT( sout );

    MarsContext* mctxt = reinterpret_cast<MarsContext*>(ctxt);

    if(!mctxt->frozen()) mctxt->freeze();

    try
    {
        Buffer b(const_cast<void*>(buffin), sin, false);

        FieldSet::Ptr fs_inp( new FieldSet(b) );                ///< @todo create a fieldset from a buffer

        Params::Ptr inparams( new FieldContext(fs_inp) );
        mctxt->set_runtime( inparams );

        Interpolate interpolator( mctxt->self() );

        FieldSet::Ptr fs_out( interpolator.eval( fs_inp ) );
        ASSERT( fs_out );

        Field& f = (*fs_out)[0];  // NOTE: we assume only one field as output

        GribHandle::Ptr gh = Grib::write(f);

        *sout = gh->write( mctxt->buffer() );

        *buffout = mctxt->buffer();

        ASSERT( *buffout );
    }
    catch( eckit::Exception& e )
    {
        std::cout << e.what() << std::endl;
        return MIR_FAILED;
    }

    return MIR_SUCCESS;
}

mir_err mir_destroy_context(mir_context_ptr ctxt)
{
    if(!ctxt) return MIR_INVALID_CONTEXT;

    MarsContext* mctxt = reinterpret_cast<MarsContext*>(ctxt);

/// @todo this fails with memory fault, we probably have memory corruption somewhere...
//    if( mctxt )
//        delete mctxt;

    return MIR_SUCCESS;
}

};

//------------------------------------------------------------------------------------------------------
