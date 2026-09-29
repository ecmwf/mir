// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
 * `RawInput`, `GribMemoryInput` and `GribDataHandleInput` read through memory
 * or a handle they do not own, and are `final` in mir, so that storage is held
 * here instead, declared before `input_` so it outlives it. Unused by the other
 * constructors. Likewise, `MultiDimensionalInput` takes over the inputs of
 * appended components but not the storage they read, so the emptied component
 * wrappers are kept here.
 */
class MIRInput final {
    std::vector<std::unique_ptr<MIRInput>> components_;
    std::unique_ptr<eckit_bridge::DataHandleWrapper> handle_;
    std::vector<double> values_;
    std::vector<unsigned char> message_;
    Parametrisation metadata_;
    std::unique_ptr<mir::input::MIRInput> input_;

public:
    bool next();

    size_t dimensions() const;

    /// Only for an input built by `from_components`.
    void append(std::unique_ptr<MIRInput> component);

    mir::input::MIRInput& inner() { return *input_; }
    const mir::input::MIRInput& inner() const { return *input_; }

    // ============== Constructors ==============

    static std::unique_ptr<MIRInput> from_data_handle(std::unique_ptr<eckit_bridge::DataHandleWrapper> handle);

    static std::unique_ptr<MIRInput> from_grib_file(rust::Str path);

    /// The message is copied in.
    static std::unique_ptr<MIRInput> from_grib_memory(rust::Slice<const uint8_t> message);

    static std::unique_ptr<MIRInput> from_multi_dimensional_grib_file(rust::Str path, size_t dimensions, size_t skip);

    /// Empty until components are appended.
    static std::unique_ptr<MIRInput> from_components();

    /// An artificial field described by a gridspec, with no data behind it.
    static std::unique_ptr<MIRInput> from_gridspec(rust::Str gridspec, bool gridded);

    /// Values and metadata are both copied in.
    static std::unique_ptr<MIRInput> from_raw(rust::Slice<const double> values, const Parametrisation& metadata);
};


}  // namespace mir_bridge
