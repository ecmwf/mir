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

#include <regex>
#include <string>


namespace mir::util {


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

    Regex(const Regex&) = delete;
    Regex(Regex&&)      = delete;

    // -- Destructor

    virtual ~Regex() = default;

    // -- Convertors
    // None

    // -- Operators

    void operator=(const Regex&) = delete;
    void operator=(Regex&&)      = delete;


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


}  // namespace mir::util
