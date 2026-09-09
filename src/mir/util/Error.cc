// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/Error.h"

#include <cerrno>
#include <cstring>
#include <ostream>


namespace mir::util {


void Error::print(std::ostream& s) const {
    int n = errno;
    if (n != 0) {
        s << "errno=" << n << ", strerror='" << std::strerror(n) << "'";
    }
}


}  // namespace mir::util
