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
