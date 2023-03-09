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


namespace eckit {
class PathName;
}
namespace mir::param {
class MIRParametrisation;
class SimpleParametrisation;
}  // namespace mir::param


namespace mir::grib {


class Config {
public:
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    Config(const eckit::PathName&, bool silent);

    Config(const Config&) = delete;
    Config(Config&&)      = delete;

    // -- Destructor

    ~Config();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const Config&) = delete;
    void operator=(Config&&)      = delete;

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
    bool silent_;

    // -- Methods

    void readConfigurationFiles(const eckit::PathName&);
    void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Config& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::grib
