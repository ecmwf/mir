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
