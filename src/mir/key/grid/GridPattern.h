// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <string>

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Regex.h"


namespace mir::key::grid {
class Grid;
}  // namespace mir::key::grid


namespace mir::key::grid {


class GridPattern {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GridPattern(const GridPattern&) = delete;
    GridPattern(GridPattern&&)      = delete;

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators

    void operator=(const GridPattern&) = delete;
    void operator=(GridPattern&&)      = delete;

    // -- Methods

    static void list(std::ostream&);
    static std::string match(const std::string& name);
    static const Grid* lookup(const std::string& name);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    explicit GridPattern(const std::string& pattern);
    virtual ~GridPattern();

    // -- Members

    const std::string pattern_;
    const util::Regex regex_;

    // -- Methods

    virtual void print(std::ostream&) const                      = 0;
    virtual const Grid* make(const std::string&) const           = 0;
    virtual std::string canonical(const std::string& name) const = 0;

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


}  // namespace mir::key::grid
