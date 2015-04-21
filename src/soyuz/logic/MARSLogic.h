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


#ifndef MARSLogic_H
#define MARSLogic_H

#include "soyuz/logic/MIRLogic.h"


class MARSLogic : public MIRLogic {
  public:

// -- Exceptions
    // None

// -- Contructors

    MARSLogic(const MIRParametrisation&);

// -- Destructor

    ~MARSLogic(); // Change to virtual if base class

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

    MARSLogic(const MARSLogic&);
    MARSLogic& operator=(const MARSLogic&);

// -- Members

// -- Methods
    // None

// -- Overridden methods

    virtual void prepare(std::vector<std::auto_ptr< mir::action::Action > >&) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    // friend std::ostream& operator<<(std::ostream& s, const MARSLogic& p)
    // { p.print(s); return s; }

};


#endif

