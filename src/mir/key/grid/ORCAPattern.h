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


#ifndef mir_key_grid_ORCAPattern_h
#define mir_key_grid_ORCAPattern_h

#include "mir/key/grid/GridPattern.h"


namespace mir {
namespace key {
namespace grid {


class ORCAPattern : public GridPattern {
public:
    // -- Exceptions
    // None

    // -- Constructors

    ORCAPattern(const std::string& name);
    ORCAPattern(const ORCAPattern&) = delete;

    // -- Destructor

    ~ORCAPattern() override;

    // -- Convertors
    // None

    // -- Operators

    ORCAPattern& operator=(const ORCAPattern&) = delete;

    // -- Methods

    static std::string sane_name(const std::string&);

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

    void print(std::ostream&) const override;
    const Grid* make(const std::string& name, const param::MIRParametrisation&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace grid
}  // namespace key
}  // namespace mir


#endif
