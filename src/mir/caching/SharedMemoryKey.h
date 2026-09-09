// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <sys/ipc.h>


namespace eckit {
class PathName;
}


namespace mir::caching {


key_t shared_memory_key(const eckit::PathName&);


}  // namespace mir::caching
