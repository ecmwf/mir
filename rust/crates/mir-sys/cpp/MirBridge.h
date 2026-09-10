/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


/*
 * Umbrella header pulled in by the cxx-generated bridge, through
 * `include!("MirBridge.h")` in lib.rs. The declarations themselves live in the
 * per-topic headers included below.
 */
#pragma once

/*
 * To report mir's exceptions as a Rust `Result` instead of aborting, the
 * cxx-generated code needs `rust::behavior::trycatch` defined, with one catch
 * block per mir exception, in the same translation unit. The generated
 * exceptions header carries that definition, so it is included here.
 *
 * It can only be defined once per translation unit, which is why it is pulled
 * in at this level: mir-sys is a leaf crate, so no other bindings crate can
 * bring a second definition in alongside it.
 */
#include "mir_exceptions.h"

#include "Job.h"
#include "LibMir.h"
#include "MIRInput.h"
#include "MIROutput.h"
#include "Parametrisation.h"
