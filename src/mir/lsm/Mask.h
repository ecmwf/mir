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
#include <vector>


namespace eckit {
class MD5;
class PathName;
}  // namespace eckit

namespace mir {
namespace param {
class MIRParametrisation;
}
namespace repres {
class Representation;
}
}  // namespace mir


namespace mir::lsm {


class Mask {
public:
    // -- Constructors

    Mask();

    Mask(const Mask&) = delete;
    Mask(Mask&&)      = delete;

    // -- Destructor

    virtual ~Mask();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const Mask&) = delete;
    void operator=(Mask&&)      = delete;

    // -- Methods

    virtual bool active() const    = 0;
    virtual bool cacheable() const = 0;
    virtual void hash(eckit::MD5&) const;
    virtual const std::vector<bool>& mask() const = 0;

    virtual std::string cacheName() const = 0;


    // Cache key takes the interpolation method into account
    static void hashCacheKey(eckit::MD5&, const eckit::PathName&, const param::MIRParametrisation&,
                             const repres::Representation&, const std::string& which);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static Mask& lookupInput(const param::MIRParametrisation&, const repres::Representation&);
    static Mask& lookupOutput(const param::MIRParametrisation&, const repres::Representation&);

    static bool sameInput(const param::MIRParametrisation&, const param::MIRParametrisation&);
    static bool sameOutput(const param::MIRParametrisation&, const param::MIRParametrisation&);

protected:
    // -- Members
    // None

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

    static Mask& lookup(const param::MIRParametrisation&, const repres::Representation&, const std::string& which);

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Mask& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::lsm
