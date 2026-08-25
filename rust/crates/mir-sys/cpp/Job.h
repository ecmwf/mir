#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "rust/cxx.h"

#include "mir/api/MIRJob.h"

#include "MIRInput.h"
#include "MIROutput.h"


namespace mir_bridge {


/**
 * A description of the transformation to apply, not the transformation itself:
 * a bag of key/value settings that mir compiles into an action plan on each
 * execute.
 *
 * The job is reusable and can be applied to any number of inputs and outputs.
 * The input is not: it is consumed by `next()`, which `execute_all` calls until
 * the stream is drained, and which the caller drives itself when using
 * `execute_one`. The bridge exposes no rewind, so a second pass needs a fresh
 * input.
 *
 * Derives from `mir::api::MIRJob` so it can be passed straight to mir, and so
 * that `set` and `clear` keep resolving key aliases (`gridname` becomes `grid`)
 * instead of bypassing them. The members below shadow inherited overload sets
 * so Rust passes `&str` and `&[f64]` rather than building `CxxString` and
 * `CxxVector` at every call site.
 */
class Job final : public mir::api::MIRJob {
public:
    void set_str(rust::Str name, rust::Str value);
    void set_f64(rust::Str name, double value);
    void set_i64(rust::Str name, int64_t value);
    void set_bool(rust::Str name, bool value);
    void set_f64_list(rust::Str name, rust::Slice<const double> values);
    void set_i64_list(rust::Str name, rust::Slice<const int64_t> values);
    void set_str_list(rust::Str name, const rust::Vec<rust::String>& values);

    void set_from_string(rust::Str args);

    void clear_key(rust::Str name);

    rust::String to_json() const;

    void execute_one(MIRInput& input, MIROutput& output) const;

    size_t execute_all(MIRInput& input, MIROutput& output) const;

    // ============== Factories ==============

    static std::unique_ptr<Job> make();
};


}  // namespace mir_bridge
