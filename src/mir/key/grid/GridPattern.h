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


#ifndef mir_key_grid_GridPattern_h
#define mir_key_grid_GridPattern_h

#include <iosfwd>
#include <string>

#include "eckit/utils/Regex.h"

#include "mir/param/SimpleParametrisation.h"


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


class GridPattern {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GridPattern(const GridPattern&) = delete;

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators

    void operator=(const GridPattern&) = delete;

    // -- Methods

    static void list(std::ostream&);
    static bool match(const std::string& name);
    static const Grid* lookup(const std::string& name,
                              const param::MIRParametrisation& = param::SimpleParametrisation());

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    GridPattern(const std::string&);
    virtual ~GridPattern();

    // -- Members

    const std::string pattern_;
    const eckit::Regex regex_;

    // -- Methods

    virtual void print(std::ostream&) const                                                         = 0;
    virtual const Grid* make(const std::string& name, const param::MIRParametrisation& param) const = 0;

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

    friend std::ostream& operator<<(std::ostream& s, const GridPattern& p) {
        p.print(s);
        return s;
    }
};


}  // namespace grid
}  // namespace key
}  // namespace mir


#endif
