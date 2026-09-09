// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/caching/SharedMemoryKey.h"

#include <sstream>

#include "eckit/filesystem/PathName.h"
#include "eckit/os/Stat.h"

#include "mir/util/Error.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::caching {


key_t shared_memory_key(const eckit::PathName& path) {
    const auto name = path.realName().asString();

    // Use time of creation epoch as proj_id for ftok to add 8 more bits of entropy
    eckit::Stat::Struct s;
    SYSCALL(eckit::Stat::stat(name.c_str(), &s));

    const auto proj_id = static_cast<int>(
#if defined(HAS_SYS_STAT_ST_CTIMESPEC)
        s.st_ctimespec.tv_sec
#else
        s.st_ctim.tv_sec
#endif
    );

    if (key_t key = ::ftok(name.c_str(), proj_id); key != -1) {
        return key;
    }

    std::ostringstream ss;
    ss << "shared_memory_key: ::ftok(" << name << "), " << util::Error();
    const auto msg = ss.str();

    Log::error() << msg << std::endl;
    throw exception::FailedSystemCall(msg);
}


}  // namespace mir::caching
