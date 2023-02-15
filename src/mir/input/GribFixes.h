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
#include <utility>
#include <vector>


namespace mir::param {
class MIRParametrisation;
class SimpleParametrisation;
}  // namespace mir::param


namespace mir::input {


class GribFixes {
public:
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    GribFixes();

    // -- Destructor

    ~GribFixes();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    const param::SimpleParametrisation& find(const param::MIRParametrisation&) const;

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
    // -- Types

    using fix_t = std::pair<param::SimpleParametrisation*, param::SimpleParametrisation*>;

    // -- Members

    std::vector<fix_t> fixes_;

    // -- Methods

    void readConfigurationFiles();
    void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const GribFixes& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::input
