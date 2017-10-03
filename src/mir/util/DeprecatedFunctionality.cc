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

#include <array>
#include <set>
#include "eckit/utils/MD5.h"


namespace mir {
namespace util {


namespace {
typedef std::array<unsigned char, MD5_DIGEST_LENGTH> message_digest_t;
static std::set<message_digest_t> known_warnings;
}  // (anonymous namespace)


DeprecatedFunctionality::DeprecatedFunctionality(const std::string& msg, std::ostream& out) {

    // only log warnings once
    message_digest_t digest;
    eckit::MD5(msg).numericalDigest(digest.data());

    const bool newWarning = known_warnings.insert(digest).second;
    if (newWarning) {
        message(msg, out);
    }
}


void DeprecatedFunctionality::message(const std::string& msg, std::ostream& out) {
    out << "DeprecatedFunctionality: " << msg << std::endl;
}


}  // namespace util
}  // namespace mir
