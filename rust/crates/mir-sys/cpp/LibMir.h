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

#include "rust/cxx.h"


namespace mir_bridge {


/// Static accessors for `mir::LibMir::instance`. Stateless; a type only
/// because every bridge entry point has to be a member.
class LibMir final {
public:
    static rust::String version();
    static rust::String git_sha1();
    static rust::String home_dir();
    static rust::String cache_dir();
    static bool caching();
};


}  // namespace mir_bridge
