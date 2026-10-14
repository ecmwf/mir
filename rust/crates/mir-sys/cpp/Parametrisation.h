// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>

#include "rust/cxx.h"

#include "mir/param/SimpleParametrisation.h"

#include "Settings.h"


namespace mir_bridge {


/**
 * The metadata carried alongside `RawInput` values, and filled in by
 * `ResizableOutput` with the grid a field was interpolated onto.
 *
 * Derives from `mir::param::SimpleParametrisation` so it can be passed
 * straight to mir, and because `json` is protected on the base.
 */
class Parametrisation final : public mir::param::SimpleParametrisation, public Settings<Parametrisation> {
public:
    /// e.g. `{"area":[1,-1,-1,1],"grid":[2,2]}`.
    rust::String to_json() const;

    // ============== Constructors ==============

    static std::unique_ptr<Parametrisation> make();
};


}  // namespace mir_bridge
