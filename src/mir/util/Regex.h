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


#pragma once

#if 1
// For Intel compilers
#include "eckit/utils/Regex.h"


namespace mir {
namespace util {
using Regex = eckit::Regex;
}
}  // namespace mir


#else


#include <regex>
#include <string>


namespace mir {
namespace util {


class Regex {
public:
    // -- Types

    using regex_t = std::regex;

    struct match_t : std::smatch {
        operator bool() const { return !std::smatch::empty(); }
    };

    // -- Exceptions
    // None

    // -- Constructors

    Regex(const std::string& pattern);

    // -- Destructor

    virtual ~Regex() = default;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    const std::string& pattern() const;
    match_t match(const std::string&) const;

    static match_t match(const std::string& pattern, const std::string& s) { return Regex(pattern).match(s); }

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Constructors

    Regex(const Regex&) = delete;

    // -- Operators

    void operator=(const Regex&) = delete;

    // -- Members

    const std::string pattern_;
    regex_t regex_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace util
}  // namespace mir


#endif
