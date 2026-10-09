// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <string>

#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace repres {
class Representation;
}
namespace util {
class Rotation;
}
}  // namespace mir


namespace mir::key::grid {


class Grid {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Grid(const Grid&) = delete;
    Grid(Grid&&)      = delete;

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators

    Grid& operator=(const Grid&) = delete;
    Grid& operator=(Grid&&)      = delete;

    // -- Methods

    virtual const repres::Representation* representation() const;
    virtual const repres::Representation* representation(const util::Rotation&) const;
    virtual const repres::Representation* representation(const param::MIRParametrisation&) const;

    virtual void parametrisation(const std::string& grid, param::SimpleParametrisation&) const;
    virtual size_t gaussianNumber() const;
    virtual std::string gridname() const;
    virtual bool isGaussian() const;  // non-rotated Gaussian grid

    static size_t default_gaussian_number() { return 64; }
    static std::string canonical(const std::string& name, const param::MIRParametrisation&);  // empty if unknown
    static bool get(const std::string& key, std::string& value, const param::MIRParametrisation&);
    static const Grid& lookup(const std::string& key);

    static void list(std::ostream&);

    const std::string& key() const { return key_; }
    const std::string& type() const { return type_; }

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Types

    enum grid_t
    {
        named_t,
        typed_t,
        regular_ll_t
    };

    // -- Constructors

    Grid(const std::string& key, const std::string& type);

    // -- Destructor

    virtual ~Grid();

    // -- Members

    const std::string key_;
    const std::string type_;

    // -- Methods

    virtual void print(std::ostream&) const = 0;

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

    friend std::ostream& operator<<(std::ostream& s, const Grid& p) {
        p.print(s);
        return s;
    }
};


// Target grid, as set by the user
struct Target {
    explicit Target(const param::MIRParametrisation&);

    std::string type;  // Grid::type(), "reduced-gg", "regular-gg", "octahedral-gg", "reduced-gg-pl-given", "griddef",
                       // "points", or empty (not set)
    std::string grid;  // canonical grid name, if set by 'grid'
    bool gaussian       = false;
    bool rotated        = false;
    long gaussianNumber = 0;  // limited by the spectral input truncation, 0 if not applicable
};


}  // namespace mir::key::grid
