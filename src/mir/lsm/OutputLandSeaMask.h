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


#ifndef OutputLandSeaMask_H
#define OutputLandSeaMask_H

#include "mir/lsm/LandSeaMask.h"

namespace mir {
namespace lsm {


class OutputLandSeaMask : public LandSeaMask {
  public:

// -- Exceptions
    // None

// -- Contructors

    OutputLandSeaMask(const std::string& name);

// -- Destructor

    virtual ~OutputLandSeaMask(); // Change to virtual if base class

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

    static const OutputLandSeaMask& lookup(const param::MIRParametrisation& parametrisation);

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

    OutputLandSeaMask(const OutputLandSeaMask&);
    OutputLandSeaMask& operator=(const OutputLandSeaMask&);

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

