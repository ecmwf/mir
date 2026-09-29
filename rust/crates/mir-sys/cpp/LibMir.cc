// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "LibMir.h"

#include "eckit/system/Library.h"

#include "mir/config/LibMir.h"


namespace mir_bridge {


rust::String LibMir::version() {
    return rust::String(mir::LibMir::instance().version());
}


rust::String LibMir::git_sha1() {
    const eckit::system::Library& lib = mir::LibMir::instance();
    return rust::String(lib.gitsha1());
}


rust::String LibMir::home_dir() {
    return rust::String(mir::LibMir::homeDir());
}


rust::String LibMir::cache_dir() {
    return rust::String(mir::LibMir::cacheDir());
}


bool LibMir::caching() {
    return mir::LibMir::caching();
}


}  // namespace mir_bridge
