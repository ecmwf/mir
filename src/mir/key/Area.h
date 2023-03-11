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

#include <iosfwd>
#include <string>


namespace mir {
namespace param {
class MIRParametrisation;
}
namespace util {
class BoundingBox;
}  // namespace util
}  // namespace mir


namespace mir::key {


class Area {
public:
    // -- Types

    struct Mode {
        static void list(std::ostream&);
    };

    // -- Constructors

    Area() = delete;

    // -- Destructor
    // None

    // -- Exceptions
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    static bool get(const param::MIRParametrisation&, util::BoundingBox&);
    static std::string action(const param::MIRParametrisation&);
    static bool match(const std::string&, util::BoundingBox&);
    static void list(std::ostream&);

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
    // None

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


}  // namespace mir::key
