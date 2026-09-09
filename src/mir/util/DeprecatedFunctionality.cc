// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/DeprecatedFunctionality.h"

#include <set>

#include "eckit/utils/MD5.h"


namespace mir::util {


static std::set<eckit::Hash::digest_t> known_messages;


DeprecatedFunctionality::DeprecatedFunctionality(const std::string& msg, Log::Channel& out) {

    // only log DeprecatedFunctionality messages once
    const eckit::Hash::digest_t digest = eckit::MD5(msg).digest();
    if (known_messages.insert(digest).second) {
        message(msg, out);
    }
}


void DeprecatedFunctionality::message(const std::string& msg, Log::Channel& out) {
    out << "DeprecatedFunctionality: " << msg << std::endl;
}


}  // namespace mir::util
