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
