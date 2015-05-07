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


#ifndef InputLandSeaMask_H
#define InputLandSeaMask_H

#include "mir/lsm/LandSeaMask.h"

namespace mir {
namespace lsm {


class InputLandSeaMask : public LandSeaMask {
  public:

// -- Exceptions
    // None

// -- Contructors

    InputLandSeaMask(const std::string& name);

// -- Destructor

    virtual ~InputLandSeaMask(); // Change to virtual if base class

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

    static const InputLandSeaMask& lookup(const param::MIRParametrisation& parametrisation, const atlas::Grid &grid);

  protected:

// -- Members


// -- Methods


// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    InputLandSeaMask(const InputLandSeaMask&);
    InputLandSeaMask& operator=(const InputLandSeaMask&);

// -- Members


// -- Methods
    // None

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends



};


}  // namespace logic
}  // namespace mir
#endif

