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


#ifndef ProdgenStyle_H
#define ProdgenStyle_H

#include "mir/style/ECMWFStyle.h"


namespace mir {
namespace style {


class ProdgenStyle : public ECMWFStyle {
  public:

// -- Exceptions
    // None

// -- Contructors

    ProdgenStyle(const param::MIRParametrisation&);

// -- Destructor

    ~ProdgenStyle(); // Change to virtual if base class

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
    // None

// -- Methods

    void print(std::ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    ProdgenStyle(const ProdgenStyle&);
    ProdgenStyle& operator=(const ProdgenStyle&);

// -- Members

// -- Methods

// -- Overridden methods

    void sh2sh(action::ActionPlan& plan) const;
    void sh2grid(action::ActionPlan& plan) const;
    virtual void grid2grid(action::ActionPlan&) const;


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    // friend std::ostream& operator<<(std::ostream& s, const ProdgenStyle& p)
    // { p.print(s); return s; }

};


}  // namespace style
}  // namespace mir
#endif

