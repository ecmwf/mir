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


#ifndef DefaultParametrisation_H
#define DefaultParametrisation_H

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



    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members


    // -- Methods

    virtual void print(std::ostream&) const;


    // -- Overridden methods

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Types

    // No copy allowed

    DefaultParametrisation(const DefaultParametrisation&);
    DefaultParametrisation& operator=(const DefaultParametrisation&);

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


};


}  // namespace param
}  // namespace mir


#endif

