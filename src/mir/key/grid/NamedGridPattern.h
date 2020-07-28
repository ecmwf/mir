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


#ifndef mir_key_grid_NamedGridPattern_h
#define mir_key_grid_NamedGridPattern_h

#include <iosfwd>
#include <string>

#include "eckit/utils/Regex.h"


namespace mir {
namespace key {
namespace grid {
class Grid;
}
}  // namespace key
}  // namespace mir


namespace mir {
namespace key {
namespace grid {


class NamedGridPattern {
public:
    // -- Exceptions
    // None

    // -- Constructors

    NamedGridPattern(const NamedGridPattern&) = delete;

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators

    void operator=(const NamedGridPattern&) = delete;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static bool match(const std::string&);
    static const Grid* build(const std::string&);
    static void list(std::ostream&);

protected:
    NamedGridPattern(const std::string&);
    virtual ~NamedGridPattern();

    // -- Members

    eckit::Regex pattern_;

    // -- Methods

    virtual const Grid* make(const std::string&) const = 0;
    virtual void print(std::ostream&) const            = 0;

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

    friend std::ostream& operator<<(std::ostream& s, const NamedGridPattern& p) {
        p.print(s);
        return s;
    }
};


}  // namespace grid
}  // namespace key
}  // namespace mir


#endif
