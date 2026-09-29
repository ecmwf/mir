// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "rust/cxx.h"


namespace mir_bridge {


/// Static accessors for `mir::LibMir::instance`, grouped under a type so Rust
/// calls them as `LibMir::version()`.
class LibMir final {
public:
    static rust::String version();
    static rust::String git_sha1();
    static rust::String home_dir();
    static rust::String cache_dir();
    static bool caching();
};


}  // namespace mir_bridge
