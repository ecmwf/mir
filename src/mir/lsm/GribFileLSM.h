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

#include "mir/lsm/LandSeaMask.h"

#include <iosfwd>



namespace mir {
namespace lsm {


class GribFileLSM : public LandSeaMask {
  public:

// -- Exceptions
    // None

// -- Contructors

    GribFileLSM(const std::string& name, const std::string& key, const param::MIRParametrisation& param, const atlas::Grid& grid);
    GribFileLSM(const std::string& name, const std::string& key,
        const param::MIRParametrisation& param, const atlas::Grid& grid, const std::string& path);

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
    // None

  protected:

// -- Members

// -- Methods


    virtual void print(std::ostream&) const; // Change to virtual if base class

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
    // None

// -- Methods

    void init(const param::MIRParametrisation& param, const atlas::Grid& grid, const std::string& path);

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

