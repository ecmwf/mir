// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cstddef>


namespace mir::param {
class MIRParametrisation;
}  // namespace mir::param


namespace mir::util {


class Wind {
public:
    // -- Exceptions
    // None

    // -- Constructors
    // None

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    static void paramIds(const param::MIRParametrisation&, long& u, long& v);

    // -- Overridden methods
    // None

    // -- Class members

    struct Defaults {
        Defaults();
        const long u;
        const long v;
        const long vo;
        const long d;
    };

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


}  // namespace mir::util
