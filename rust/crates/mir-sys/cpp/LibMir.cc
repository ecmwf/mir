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


#include "LibMir.h"

#include "mir/config/LibMir.h"


namespace mir_bridge {


namespace {
// Full 40-character hash, matching what the Python bindings report.
constexpr unsigned int SHA1_LENGTH = 40;
}  // namespace


rust::String LibMir::version() {
    return rust::String(mir::LibMir::instance().version());
}


rust::String LibMir::git_sha1() {
    return rust::String(mir::LibMir::instance().gitsha1(SHA1_LENGTH));
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
