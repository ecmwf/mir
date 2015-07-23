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

#include "eckit/config/Resource.h"
#include "eckit/io/Buffer.h"
#include "eckit/log/CallbackChannel.h"
#include "eckit/memory/NonCopyable.h"
#include "eckit/runtime/Context.h"
#include "eckit/runtime/LibBehavior.h"
#include "eckit/value/Properties.h"

#include "atlas/FieldSet.h"
#include "atlas/io/Grib.h"

#include "mir/mars_api.h"

#include "mir/Context.h"
#include "mir/Interpolate.h"
#include "mir/Params.h"
#include "mir/FieldParams.h"

using namespace eckit;
using namespace eckit::grib;
using namespace atlas;
using namespace atlas::io;
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

    Params::value_t operator[]( const Params::key_t& k ) const
    {
        if( values_.has(k) )
            return values_[k];
        else
            return get( *this, k );
    }

private: // members

    friend void print( const MarsParams& p, std::ostream& s );
    const Params& values_;
};

void print( const MarsParams& p, std::ostream& s ) {
    s << p.values_;
}

//------------------------------------------------------------------------------------------------------

class MarsContext {

public: // methods

    MarsContext() :
        frozen_(false),
        mirContext_( new MirContext() ),
        buffer_( Resource<size_t>( "MirFieldBufferSize;$MIR_FIELD_BUFFER_SIZE",
                                   60*1024*1024) )
    {
        MarsParams mars( (Params( mars_values_ )) ); // Most Vexing Parse

        mirContext_->params().push_front( Params(mars) );
    }

    virtual ~MarsContext()
    {
        std::cout << "Destroying MarsContext" << std::endl;
    }

    bool frozen() const { return frozen_; }
    void freeze() { frozen_ = true; }

    ValueParams& mars_values() { return mars_values_; }
    MirParams& mir_params() { return mirContext_->params(); }

    Buffer& buffer() { return buffer_; }

private: // members

    bool frozen_;

    ValueParams mars_values_;

    ScopedPtr<MirContext> mirContext_;

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

mir_err mir_interpolate(mir_context_ptr ctxt, const void* buffin, size_t sin,
                        void **buffout, size_t* sout)
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

        ///< @todo create a fieldset from a buffer
        FieldSet::Ptr fs_inp( new FieldSet(b) );

        MirParams params_mir = mctxt->mir_params();
        Params params_inp( ScopeParams( "Input", Params(FieldParams(fs_inp)) ) );

        params_mir.push_front( params_inp );

        Interpolate interpolator( (Params(params_mir)) ); // C++ Most Vexing Parse

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
