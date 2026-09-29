// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cstddef>
#include <memory>

#include "rust/cxx.h"

#include "mir/api/MIRJob.h"

#include "MIRInput.h"
#include "MIROutput.h"
#include "Settings.h"


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
 * input. Inputs carrying a single message, such as `GribMemoryInput`, do not
 * implement `next()` at all and have to go through `execute_one`.
 *
 * Derives from `mir::api::MIRJob` so it can be passed straight to mir, and so
 * that `set` and `clear` keep resolving key aliases (`gridname` becomes `grid`)
 * instead of bypassing them.
 */
class Job final : public mir::api::MIRJob, public Settings<Job> {
public:
    void set_from_string(rust::Str args);

    rust::String to_json() const;

    void execute_one(MIRInput& input, MIROutput& output) const;

    size_t execute_all(MIRInput& input, MIROutput& output) const;

    // ============== Constructors ==============

    static std::unique_ptr<Job> make();
};


}  // namespace mir_bridge
