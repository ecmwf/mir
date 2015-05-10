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
/// @date Apr 2015


#ifndef GribFileLSM_H
#define GribFileLSM_H

#include <iosfwd>

#include "eckit/filesystem/PathName.h"

#include "mir/lsm/Mask.h"

namespace mir {
namespace param {
class MIRParametrisation;
}
}

namespace mir {
namespace lsm {

class GribFileLSM : public Mask {
  public:

// -- Exceptions
    // None

// -- Contructors

    GribFileLSM(const std::string& name, const eckit::PathName& path,
                const param::MIRParametrisation& parametrisation,
                const atlas::Grid& grid, const std::string& which);

// -- Destructor

    virtual ~GribFileLSM(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods

    static void hashCacheKey(eckit::MD5&, const eckit::PathName& path,
                const param::MIRParametrisation& parametrisation,
                const atlas::Grid& grid,
                const std::string& which);

  protected:

// -- Members

// -- Methods

    virtual void hash(eckit::MD5&) const;

    virtual void print(std::ostream&) const;

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    GribFileLSM(const GribFileLSM&);
    GribFileLSM& operator=(const GribFileLSM&);

// -- Members

    eckit::PathName path_;

// -- Methods


// -- Overridden methods


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends


};



}  // namespace logic
}  // namespace mir
#endif

