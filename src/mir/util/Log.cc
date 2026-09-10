// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/Log.h"

#include <ostream>

#include "eckit/config/Resource.h"
#include "eckit/log/BigNum.h"

#include "mir/config/LibMir.h"


namespace mir {


Log::Channel& Log::debug() {
    return eckit::Log::debug<LibMir>();
}


bool Log::debug_active() {
    static const bool active = eckit::LibResource<bool, LibMir>("$MIR_DEBUG", false);
    return active;
}


void Log::Pretty::print(std::ostream& s) const {
    s << eckit::BigNum(count_);
    if (plural_) {
        s << ' ' << plural_(count_);
    }
}


}  // namespace mir
