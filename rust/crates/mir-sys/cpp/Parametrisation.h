#pragma once

#include <cstdint>
#include <memory>

#include "rust/cxx.h"

#include "mir/param/SimpleParametrisation.h"

namespace mir_bridge {

//----------------------------------------------------------------------------------------------------------------------

/**
 * The metadata carried alongside `RawInput` values, and filled in by
 * `ResizableOutput` with the grid a field was interpolated onto.
 *
 * Derives from `mir::param::SimpleParametrisation` so it can be passed
 * straight to mir, and because `json` is protected on the base. The setters
 * shadow inherited overloads so Rust passes `&str` and `&[i64]` rather than
 * building `CxxString` and `CxxVector` at every call site.
 */
class Parametrisation final : public mir::param::SimpleParametrisation {
public:

    void set_str(rust::Str name, rust::Str value);
    void set_f64(rust::Str name, double value);
    void set_i64(rust::Str name, int64_t value);
    void set_bool(rust::Str name, bool value);
    void set_f64_list(rust::Str name, rust::Slice<const double> values);
    void set_i64_list(rust::Str name, rust::Slice<const int64_t> values);

    /// e.g. `{"area":[1,-1,-1,1],"grid":[2,2]}`.
    rust::String to_json() const;

    // ============== Factories ==============

    static std::unique_ptr<Parametrisation> create();
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mir_bridge
