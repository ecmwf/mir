/*
 * (C) Copyright 1996-2014 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 *   @author Tiago Quintino
 *   @date   Jul 2014
 */

#ifndef mir_mars_api_h
#define mir_mars_api_h

#include <stddef.h>

#include "mir/mir_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mir_context  mir_context;
typedef mir_context*        mir_context_ptr;

typedef int         mir_err;

typedef void ( *logger_proc ) ( mir_context_ptr*, int, const char * );

mir_err mir_create_context( mir_context_ptr* );

mir_err mir_set_context_logger( mir_context_ptr, logger_proc );

mir_err mir_set_context_value( mir_context_ptr, const char*, const char* );

mir_err mir_interpolate( mir_context_ptr, const void *, size_t, void *const *, size_t* );

mir_err mir_destroy_context( mir_context_ptr );


#define MIR_SUCCESS                         0
#define MIR_FAILED                         -1
#define MIR_INVALID_CONTEXT                -2
#define MIR_CONTEXT_FROZEN                 -3

#ifdef __cplusplus
}
#endif

#endif

