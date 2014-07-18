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

#include "mir/mars_api.h"

#include "mir/Interpolate.h"
#include "mir/Params.h"
#include "mir/FieldContext.h"

using namespace mir;
using namespace atlas::grid;
using namespace eckit;

//------------------------------------------------------------------------------------------------------

class MarsParams : public DispatchParams<MarsParams> {

public: // methods

    MarsParams( const Params& p) : values_(p)
    {
        dispatch_["Target.Grid"] = &MarsParams::getGrid; // examples
        dispatch_["Target.Area"] = &MarsParams::getGrid;
    }

    /// this will be build with a request

    value_t getGrid( const key_t& ) const
    {
        /// translate from GRID=1/1 to ValueMap( GridType = "regular_ll", LatInc = 1, LonInc = 1 )


    }

    value_t getArea( const key_t& ) const
    {
        /// translate from AREA=1/1/1/1 to ValueMap( GridType = "regular_ll", LatInc = 1, LonInc = 1 )
    }

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
        frozen_(false)
    {
        Params::Ptr mars( new MarsParams( mars_values_ ) );

        push_front( mars->self() );
    }

    bool frozen() const { return frozen_; }
    void freeze() { frozen_ = true; }

    ValueParams& mars_values() { return mars_values_; }

    void set_runtime( Params::Ptr p ) { runtime_ = p.get(); }

private: // members

    bool frozen_;

    ValueParams mars_values_;
    Params* runtime_;
};

//------------------------------------------------------------------------------------------------------
//

extern "C" {

mir_err mir_create_context( mir_context_ptr* ctxt )
{
    DEBUG_HERE;

    LibBehavior* b = new LibBehavior();

    Context::instance().behavior( b );

    MarsContext* mctxt = new MarsContext();
    ASSERT( mctxt );
    *ctxt = reinterpret_cast<mir_context_ptr>(mctxt);

    DEBUG_HERE;

    return MIR_SUCCESS;
}

mir_err mir_set_context_logger(mir_context_ptr ctxt, logger_proc logger )
{
    dynamic_cast<CallbackChannel&>(Log::info()).register_callback( logger, &ctxt );

    return MIR_SUCCESS;
}

mir_err mir_set_context_value(mir_context_ptr ctxt, const char* key, const char* value)
{
    DEBUG_HERE;

    if(!ctxt) return MIR_INVALID_CONTEXT;

    MarsContext* mctxt = reinterpret_cast<MarsContext*>(ctxt);

    DEBUG_VAR(mctxt->frozen());

    if( mctxt->frozen() )
        return MIR_CONTEXT_FROZEN;

    DEBUG_VAR(key);
    DEBUG_VAR(value);

    mctxt->mars_values().set( key, eckit::Value(value) );

    DEBUG_HERE;

    return MIR_SUCCESS;
}

mir_err mir_interpolate(mir_context_ptr ctxt, const void* buffin, size_t sin, void ** buffout, size_t* sout)
{
    DEBUG_HERE;

    if(!ctxt) return MIR_INVALID_CONTEXT;

    MarsContext* mctxt = reinterpret_cast<MarsContext*>(ctxt);

    if(!mctxt->frozen()) mctxt->freeze();

    bool shared = false;

    try
    {
        Buffer b(const_cast<void*>(buffin), sin, shared);

        FieldSet::Ptr fs_inp( new FieldSet(b) );                ///< @todo create a fieldset from a buffer

        Params::Ptr inparams( new FieldContext(fs_inp) );
        mctxt->set_runtime( inparams );

        Interpolate interpolator( mctxt->self() );

        FieldSet::Ptr fs_out = interpolator.eval( fs_inp );

        FieldHandle::Data& f = fs_out->fields().at(0)->data();  // NOTE: we assume only one field as output
        (*buffout) = f.data();
        (*sout)    = f.extent(0);                               // NOTE: we assume that the array is with 1 rank (single column vector)

        ASSERT( fs_out );
    }
    catch( eckit::Exception& e )
    {
        std::cout << e.what() << std::endl;
        return MIR_FAILED;
    }

    DEBUG_HERE;

    return MIR_SUCCESS;
}

mir_err mir_destroy_context(mir_context_ptr ctxt)
{
    DEBUG_HERE;

    if(!ctxt) return MIR_INVALID_CONTEXT;

    MarsContext* mctxt = reinterpret_cast<MarsContext*>(ctxt);
    delete mctxt;

    DEBUG_HERE;

    return MIR_SUCCESS;
}

};

//------------------------------------------------------------------------------------------------------
