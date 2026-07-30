// mir library bridge — wraps `mir::LibMir` metadata.
#pragma once

#include "rust/cxx.h"

namespace mir_bridge {

//----------------------------------------------------------------------------------------------------------------------

/// Static accessors for the mir library itself. Holds no state; it exists as a
/// type only because every bridge entry point is a member, and because
/// `version`/`gitsha1` are instance methods reached through `LibMir::instance`.
class LibMir final {
public:

    static rust::String version();
    static rust::String git_sha1();
    static rust::String home_dir();
    static rust::String cache_dir();
    static bool caching();
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mir_bridge
