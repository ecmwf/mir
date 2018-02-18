/*
 * (C) Copyright 1996- ECMWF.
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


#ifndef mir_param_DefaultParametrisation_h
#define mir_param_DefaultParametrisation_h

#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace param {


class DefaultParametrisation : public SimpleParametrisation {
public:

    // -- Exceptions
    // None

    // -- Contructors

    DefaultParametrisation();

    // -- Destructor

    virtual ~DefaultParametrisation();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

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

private:

    // -- Types
    // None

    // -- Members

    mutable SimpleParametrisation cache_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


}  // namespace param
}  // namespace mir


#endif

