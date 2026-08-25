// mir output bridge — wraps `mir::output::MIROutput` and its factories.
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "rust/cxx.h"

#include "mir/output/MIROutput.h"

#include "Parametrisation.h"

namespace mir_bridge {

// Defined on the Rust side, cxx generates the type.
struct OutputBox;

//----------------------------------------------------------------------------------------------------------------------

/// Wraps `mir::output::MIROutput*` for Rust FFI. Takes ownership.
///
/// `ResizableOutput` writes through a vector and a parametrisation it holds by
/// reference, and `GribMemoryOutput` encodes into a caller-provided buffer.
/// Both are `final` in mir, so that storage is held here instead, declared
/// before `output_` so that it outlives it. It is unused by the other
/// factories, and the readback accessors throw unless the output held is the
/// kind that fills it.
class MIROutputWrapper final {
    std::vector<double> values_;
    std::vector<unsigned char> message_;
    Parametrisation metadata_;
    std::unique_ptr<mir::output::MIROutput> output_;

public:

    /// Interpolated values, for an output built by `to_resizable`.
    rust::Slice<const double> values() const;

    /// The grid the field was interpolated onto, for an output built by
    /// `to_resizable`, e.g. `{"area":[1,-1,-1,1],"grid":[2,2]}`.
    rust::String metadata_json() const;

    /// The encoded message, for an output built by `to_grib_memory`. Empty
    /// until a job has run.
    rust::Slice<const uint8_t> message() const;

    /// Access underlying for other C++ bridge code.
    mir::output::MIROutput& inner() { return *output_; }
    const mir::output::MIROutput& inner() const { return *output_; }

    // ============== Factories ==============

    /// Encodes each field as GRIB and hands the message to `output`.
    static std::unique_ptr<MIROutputWrapper> to_callback(rust::Box<OutputBox> output);

    /// Appends to, or truncates, a GRIB file.
    static std::unique_ptr<MIROutputWrapper> to_grib_file(rust::Str path, bool append);

    /// Encodes GRIB into an owned buffer of `capacity` bytes, readable through
    /// `message`. mir throws if the encoded message does not fit.
    static std::unique_ptr<MIROutputWrapper> to_grib_memory(size_t capacity);

    /// Collects interpolated values into an owned, growable buffer, readable
    /// through `values` and `metadata_json`.
    static std::unique_ptr<MIROutputWrapper> to_resizable();

    /// Runs the job but discards the result — useful for timing and validation.
    static std::unique_ptr<MIROutputWrapper> to_empty();
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mir_bridge
