// mir C++ bridge for Rust FFI
#pragma once

#include "eckit_bridge.h"
#include "eckit_exceptions.h"

#include "mir/api/MIRJob.h"
#include "mir/input/GribDataHandleInput.h"
#include "mir/input/VectorInput.h"
#include "mir/output/GribOutput.h"

#include "rust/cxx.h"

#include <memory>
#include <string>

namespace mir_bridge {

// Forward declaration — defined on the Rust side
struct OutputBox;

// ==================== MIRJob ====================

// ==================== GribDataHandleInput ====================

/// Wraps `mir::input::GribDataHandleInput` for Rust FFI.
class GribInputWrapper {
    std::unique_ptr<mir::input::GribDataHandleInput> input_;

public:
    explicit GribInputWrapper(eckit_bridge::DataHandleWrapper& handle);

    /// Advance to first/next message. Returns false when exhausted.
    bool next();

    /// Access underlying MIRInput (for VectorInput construction).
    mir::input::MIRInput& as_mir_input() { return *input_; }
};

std::unique_ptr<GribInputWrapper> grib_input_create(eckit_bridge::DataHandleWrapper& handle);

// ==================== VectorInput ====================

/// Wraps `mir::input::VectorInput` for Rust FFI.
class VectorInputWrapper {
    std::unique_ptr<mir::input::VectorInput> input_;

public:
    VectorInputWrapper(GribInputWrapper& component1, GribInputWrapper& component2);

    /// Access underlying MIRInput (for execute).
    mir::input::MIRInput& as_mir_input() { return *input_; }
};

std::unique_ptr<VectorInputWrapper> vector_input_create(GribInputWrapper& component1, GribInputWrapper& component2);

// ==================== MIRJob ====================

/// Wraps `mir::api::MIRJob` for Rust FFI.
class MIRJobWrapper {
    mir::api::MIRJob job_;

public:
    MIRJobWrapper() = default;

    // Set parameters
    void set_string(rust::Str name, rust::Str value);
    void set_double(rust::Str name, double value);
    void set_long(rust::Str name, int64_t value);
    void set_bool(rust::Str name, bool value);
    void set_double_list(rust::Str name, rust::Slice<const double> values);

    // Convenience: set from a "name=value" string
    void set_from_string(rust::Str args);

    // Clear a parameter
    void clear(rust::Str name);

    // Set representation from input
    void representation_from(GribInputWrapper& input);

    // Serialize to JSON string
    rust::String json_str() const;

    // Generate mir-tool CLI command
    rust::String mir_tool_call() const;

    // Execute with scalar input (single DataHandle)
    void execute(eckit_bridge::DataHandleWrapper& input, rust::Box<OutputBox> output);

    // Execute with GribInput (for scalar interpolation)
    void execute_input(GribInputWrapper& input, rust::Box<OutputBox> output);

    // Execute with VectorInput (for VO/D → U/V conversion)
    void execute_vector(VectorInputWrapper& input, rust::Box<OutputBox> output);

    // Access underlying
    mir::api::MIRJob& inner() { return job_; }
    const mir::api::MIRJob& inner() const { return job_; }
};

// Factory
std::unique_ptr<MIRJobWrapper> mir_job_create();

}  // namespace mir_bridge
