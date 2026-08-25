#include "LibMir.h"

#include "mir/config/LibMir.h"

namespace mir_bridge {

//----------------------------------------------------------------------------------------------------------------------

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

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mir_bridge
