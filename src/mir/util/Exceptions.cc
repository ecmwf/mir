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


#include "mir/util/Exceptions.h"

#include <cerrno>
#include <cstring>


namespace mir::exception {


static std::string msg_errno(const std::string& msg) {
    const int e = errno;
    return msg + " (errno=" + std::to_string(e) + ", '" + std::strerror(e) + "')";
};


FailedSystemCall::FailedSystemCall(const std::string& msg) : eckit::FailedSystemCall(msg_errno(msg)) {}


}  // namespace mir::exception
