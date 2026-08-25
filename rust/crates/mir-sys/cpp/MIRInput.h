// mir input bridge — wraps `mir::input::MIRInput` and its factories.
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "rust/cxx.h"

#include "EckitBridge.h"

#include "mir/input/MIRInput.h"

#include "Parametrisation.h"

namespace mir_bridge {

//----------------------------------------------------------------------------------------------------------------------

/// Wraps `mir::input::MIRInput*` for Rust FFI. Takes ownership.
///
/// An input is a cursor over a stream of fields, not a single field: `next`
/// advances to the following message and returns false once the stream is
/// drained. A single-field input yields exactly once.
///
/// `RawInput` reads through a values pointer and a metadata reference, and
/// `GribMemoryInput` wraps its message rather than copying it. Both are `final`
/// in mir, so the storage they read through is held here instead, declared
/// before `input_` so that it outlives it. It is unused by the other factories.
class MIRInputWrapper final {
    std::vector<double> values_;
    std::vector<unsigned char> message_;
    Parametrisation metadata_;
    std::unique_ptr<mir::input::MIRInput> input_;

public:

    /// Advance to the next message; false once the stream is exhausted.
    bool next();

    /// Fields carried per message — 1 for a scalar, 2 for a vector pair.
    size_t dimensions() const;

    /// Access underlying for other C++ bridge code.
    mir::input::MIRInput& inner() { return *input_; }
    const mir::input::MIRInput& inner() const { return *input_; }

    // ============== Factories ==============

    /// GRIB messages read from a data handle, which must outlive the input.
    static std::unique_ptr<MIRInputWrapper> from_data_handle(eckit_bridge::DataHandleWrapper& handle);

    /// GRIB messages read from a file.
    static std::unique_ptr<MIRInputWrapper> from_grib_file(rust::Str path);

    /// A single GRIB message, copied in.
    static std::unique_ptr<MIRInputWrapper> from_grib_memory(rust::Slice<const uint8_t> message);

    /// Consecutive messages read as one N-dimensional field. This is how mir
    /// pairs components for the `vod2uv` and `uv2uv` job keys.
    static std::unique_ptr<MIRInputWrapper> from_multi_dimensional_grib_file(rust::Str path, size_t dimensions,
                                                                            size_t skip);

    /// An artificial field described by a gridspec, with no data behind it.
    static std::unique_ptr<MIRInputWrapper> from_gridspec(rust::Str gridspec, bool gridded);

    /// A single field of raw values plus metadata describing its grid, both
    /// copied in.
    static std::unique_ptr<MIRInputWrapper> from_raw(rust::Slice<const double> values,
                                                     const Parametrisation& metadata);
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mir_bridge
