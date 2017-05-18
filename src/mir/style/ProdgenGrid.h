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


#ifndef ProdgenGrid_H
#define ProdgenGrid_H

#include "mir/param/DelayedParametrisation.h"
#include <vector>


namespace mir {

namespace param {
class MIRParametrisation;
}

namespace style {


class ProdgenGrid : public param::DelayedParametrisation {
  public:

// -- Exceptions
    // None

// -- Contructors

    ProdgenGrid(const param::MIRParametrisation &parametrisation);

// -- Destructor

    virtual ~ProdgenGrid(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods
    // None



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
    // virtual bool has(const std::string& name) const;

    virtual void get(const std::string& name, std::vector<double>& value) const;

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    ProdgenGrid(const ProdgenGrid&);
    ProdgenGrid& operator=(const ProdgenGrid&);

// -- Members

    const param::MIRParametrisation &parametrisation_;

// -- Methods
    // None

// -- Overridden methods
    // None


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend std::ostream& operator<<(std::ostream& s,const ProdgenGrid& p) {
        p.print(s);
        return s;
    }

};


}  // namespace param
}  // namespace mir
#endif

