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

#include "eckit/value/Properties.h"

#include "atlas/grid/FieldSet.h"

#include "mir/mars_api.h"

#include "mir/Interpolate.h"

using namespace mir;
using namespace atlas::grid;
using namespace eckit;

//------------------------------------------------------------------------------------------------------

class MarsContext : private NonCopyable {

public: // methods

    MarsContext() : frozen_(false) {}

    void set( const std::string& k, const std::string& v)
    {
        props_.set(k,v);
    }

    bool frozen() const { return frozen_; }
    void freeze() { frozen_ = true; }

    const eckit::Properties& props() const { return props_; }

private: // members

    bool frozen_;

    eckit::Properties props_;

};

//------------------------------------------------------------------------------------------------------

mir_err mir_create_context( mir_context_ptr* ctxt )
{
    MarsContext* mars_ctxt = new MarsContext();
    ASSERT(mars_ctxt);
    *ctxt = reinterpret_cast<mir_context_ptr>(mars_ctxt);
    return MIR_SUCCESS;
}

mir_err mir_set_context_logger(mir_context_ptr, logger_proc)
{
    ///< @todo regist the logger for the library
    NOTIMP;
    return MIR_SUCCESS;
}

mir_err mir_set_context_value(mir_context_ptr ctxt, const char* key, const char* value)
{
    if(!ctxt) return MIR_INVALID_CONTEXT;

    MarsContext* mctxt = reinterpret_cast<MarsContext*>(ctxt);

    if( mctxt->frozen() )
        return MIR_CONTEXT_FROZEN;

    mctxt->set(key,value);

    return MIR_SUCCESS;
}

mir_err mir_interpolate(mir_context_ptr ctxt, const void* buffin, size_t sin, void **const buffout, size_t* sout)
{
    if(!ctxt) return MIR_INVALID_CONTEXT;

    MarsContext* mctxt = reinterpret_cast<MarsContext*>(ctxt);

    if(!mctxt->frozen()) mctxt->freeze();

    try
    {
        FieldSet::Ptr fs_inp; ///< @todo create a fieldset from a buffer

        Interpolate interpolator( mctxt->props() );

        FieldSet::Ptr fs_out = interpolator.eval( fs_inp );

        (*buffout) = NULL; ///< @todo fill in the output buffer from the fieldset

        ASSERT( fs_out );
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
    delete mctxt;
    return MIR_SUCCESS;
}

//------------------------------------------------------------------------------------------------------
