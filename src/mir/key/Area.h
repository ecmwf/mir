// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
