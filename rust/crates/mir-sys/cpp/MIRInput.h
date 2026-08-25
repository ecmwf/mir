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

/**
 * Owns a `mir::input::MIRInput`, which is a cursor over a stream of fields
 * rather than a single field, and is consumed as `next()` advances it.
 *
 * `RawInput` and `GribMemoryInput` read through memory they do not own, and
 * are `final` in mir, so that storage is held here instead, declared before
 * `input_` so it outlives it. Unused by the other factories.
 */
class MIRInput final {
    std::vector<double> values_;
    std::vector<unsigned char> message_;
    Parametrisation metadata_;
    std::unique_ptr<mir::input::MIRInput> input_;

public:
    bool next();

    size_t dimensions() const;

    mir::input::MIRInput& inner() { return *input_; }
    const mir::input::MIRInput& inner() const { return *input_; }

    // ============== Factories ==============

    /// The handle must outlive the input.
    static std::unique_ptr<MIRInput> from_data_handle(eckit_bridge::DataHandleWrapper& handle);

    static std::unique_ptr<MIRInput> from_grib_file(rust::Str path);

    /// The message is copied in.
    static std::unique_ptr<MIRInput> from_grib_memory(rust::Slice<const uint8_t> message);

    static std::unique_ptr<MIRInput> from_multi_dimensional_grib_file(rust::Str path, size_t dimensions, size_t skip);

    /// An artificial field described by a gridspec, with no data behind it.
    static std::unique_ptr<MIRInput> from_gridspec(rust::Str gridspec, bool gridded);

    /// Values and metadata are both copied in.
    static std::unique_ptr<MIRInput> from_raw(rust::Slice<const double> values, const Parametrisation& metadata);
};

}  // namespace mir_bridge
