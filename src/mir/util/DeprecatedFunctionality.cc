/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date October 2017


#include "mir/util/DeprecatedFunctionality.h"

#include "mir/config/LibMir.h"

#define SLEEP
#ifdef SLEEP
#include <unistd.h>  // for ::sleep
#endif


namespace mir {
namespace util {


DeprecatedFunctionality::DeprecatedFunctionality(const std::string& msg, std::ostream& out) {
    message(msg, out);
}

void DeprecatedFunctionality::message(const std::string& msg, std::ostream& out) {
#if defined (SLEEP)
    const eckit::Configuration& config = LibMir::instance().configuration();
    const size_t sleepTime = config.getUnsigned("deprecated-functionality-sleep", 0);

    out << "DeprecatedFunctionality: " << msg << ", sleeping for " << sleepTime << 's' << std::endl;
    ::sleep(static_cast<unsigned int>(sleepTime));
#else
    out << msg << std::endl;
#endif
}


}  // namespace util
}  // namespace mir
