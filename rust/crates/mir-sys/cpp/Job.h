// mir job bridge — wraps `mir::api::MIRJob`.
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "rust/cxx.h"

#include "mir/api/MIRJob.h"

#include "MIRInput.h"
#include "MIROutput.h"

namespace mir_bridge {

//----------------------------------------------------------------------------------------------------------------------

/// A description of the transformation to apply, not the transformation itself:
/// a bag of key/value settings that mir compiles into an action plan on each
/// execute. The same job can be applied to any number of inputs and outputs.
///
/// Derives from `mir::api::MIRJob` rather than holding one so it can be passed
/// straight to mir, and so that `set` and `clear` keep resolving key aliases
/// (`gridname` becomes `grid`) instead of bypassing them. The members below
/// shadow inherited overload sets purely for ergonomics — they take `rust::Str`
/// and `rust::Slice` so Rust passes `&str` and `&[f64]` rather than building
/// `CxxString` and `CxxVector` at every call site.
class Job final : public mir::api::MIRJob {
public:

    void set_str(rust::Str name, rust::Str value);
    void set_f64(rust::Str name, double value);
    void set_i64(rust::Str name, int64_t value);
    void set_bool(rust::Str name, bool value);
    void set_f64_list(rust::Str name, rust::Slice<const double> values);
    void set_i64_list(rust::Str name, rust::Slice<const int64_t> values);
    void set_str_list(rust::Str name, const rust::Vec<rust::String>& values);

    /// Parse `name=value` pairs the way the mir tool does; a bare `name` sets
    /// it to true.
    void set_from_string(rust::Str args);

    void clear_key(rust::Str name);

    /// Take the output grid from an input rather than stating it.
    void representation_from(const MIRInputWrapper& input);

    /// e.g. `{"grid":"F16"}`.
    rust::String to_json() const;

    /// The equivalent mir-tool command line, for debugging.
    rust::String mir_tool_call() const;

    /// Transform the message the input is currently positioned on. Callers are
    /// responsible for having advanced it; see `execute_all`.
    void execute_one(MIRInputWrapper& input, MIROutputWrapper& output) const;

    /// Drain the input, transforming every message, and return how many were
    /// processed. Single-field inputs such as `from_raw` yield one.
    size_t execute_all(MIRInputWrapper& input, MIROutputWrapper& output) const;

    // ============== Factories ==============

    static std::unique_ptr<Job> create();
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mir_bridge
