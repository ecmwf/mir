// mir C++ bridge for Rust FFI — umbrella header pulled in by the cxx-generated
// bridge (`include!("MirBridge.h")` in lib.rs). Real declarations live in the
// per-topic headers below.
#pragma once

// Unlike eckit's and metkit's umbrella headers, this one includes the generated
// exceptions header: mir-sys is a leaf crate, so there is no downstream `-sys`
// that would end up with two `rust::behavior::trycatch` specialisations in one
// translation unit, and the cxx-generated code needs the specialisation visible
// to map mir's exceptions onto `Result`.
#include "mir_exceptions.h"

#include "Job.h"
#include "LibMir.h"
#include "MIRInput.h"
#include "MIROutput.h"
#include "Parametrisation.h"
