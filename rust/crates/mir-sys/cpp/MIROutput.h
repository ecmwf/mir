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


/**
 * Owns a `mir::output::MIROutput` and the storage it writes through.
 *
 * `ResizableOutput` and `GribMemoryOutput` write through memory they do not
 * own, and are `final` in mir, so that storage is held here instead, declared
 * before `output_` so it outlives it. The readback accessors throw unless the
 * output held is the kind that fills them.
 */
class MIROutput final {
    std::vector<double> values_;
    std::vector<unsigned char> message_;
    Parametrisation metadata_;
    std::unique_ptr<mir::output::MIROutput> output_;

public:
    rust::Slice<const double> values() const;

    rust::String metadata_json() const;

    /// Empty until a job has run.
    rust::Slice<const uint8_t> message() const;

    mir::output::MIROutput& inner() { return *output_; }
    const mir::output::MIROutput& inner() const { return *output_; }

    // ============== Factories ==============

    /// Encodes each field as GRIB and hands the message to `output`.
    static std::unique_ptr<MIROutput> to_callback(rust::Box<OutputBox> output);

    /// Appends to, or truncates, a GRIB file.
    static std::unique_ptr<MIROutput> to_grib_file(rust::Str path, bool append);

    /// Encodes into an owned buffer of `capacity` bytes, read back through
    /// `message`. mir throws if the encoded message does not fit.
    static std::unique_ptr<MIROutput> to_grib_memory(size_t capacity);

    /// Collects values into an owned, growable buffer, read back through
    /// `values` and `metadata_json`.
    static std::unique_ptr<MIROutput> to_resizable();

    /// Runs the job but discards the result, for timing and validation.
    static std::unique_ptr<MIROutput> to_empty();
};


}  // namespace mir_bridge
