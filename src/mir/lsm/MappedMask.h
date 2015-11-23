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


#ifndef MappedMask_H
#define MappedMask_H

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

class MappedMask : public Mask {
  public:

// -- Exceptions
    // None

// -- Contructors

    MappedMask(const std::string& name,
                const param::MIRParametrisation& parametrisation,
                const atlas::Grid& grid, const std::string& which);

// -- Destructor

    virtual ~MappedMask(); // Change to virtual if base class

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

    MappedMask(const MappedMask&);
    MappedMask& operator=(const MappedMask&);

// -- Members

    eckit::PathName path_;
    std::vector<bool> mask_;

// -- Methods


// -- Overridden methods

    virtual const std::vector<bool> &mask() const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends


};



}  // namespace logic
}  // namespace mir
#endif

