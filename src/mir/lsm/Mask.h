/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @author Tiago Quintino
/// @date Apr 2015


#ifndef Mask_H
#define Mask_H

#include <iosfwd>
#include <string>
#include <vector>

#include "eckit/memory/NonCopyable.h"

namespace eckit {
class MD5;
}
namespace atlas {
namespace grid {
class Grid;
}
}
namespace mir {
namespace param {
class MIRParametrisation;
}
}

namespace mir {
namespace lsm {

class Mask : private eckit::NonCopyable {
  public:

    Mask(const std::string &name);

    // -- Destructor

    virtual ~Mask(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual bool active() const;

    virtual bool cacheable() const;

    virtual void hash(eckit::MD5 &) const;
    virtual const std::vector<bool> &mask() const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static  Mask &lookupInput(const param::MIRParametrisation &param, const atlas::grid::Grid &grid);
    static  Mask &lookupOutput(const param::MIRParametrisation &param, const atlas::grid::Grid &grid);

  protected:

    // -- Members

    std::string name_;

    // -- Methods

    virtual void print(std::ostream &) const = 0;


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

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    static  Mask &lookup(const param::MIRParametrisation &param,
                         const atlas::grid::Grid &grid,
                         const std::string &which);


    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const Mask &p) {
        p.print(s);
        return s;
    }

};

}  // namespace logic
}  // namespace mir

#endif

