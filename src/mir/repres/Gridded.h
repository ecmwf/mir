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


#ifndef Gridded_H
#define Gridded_H

#include "mir/repres/Representation.h"


namespace mir {

namespace util {
class MIRStatistics;
}

namespace repres {


class Gridded : public Representation {
  public:

// -- Exceptions
    // None

// -- Contructors

    Gridded(const param::MIRParametrisation&);

// -- Destructor

    virtual ~Gridded(); // Change to virtual if base class

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
    Gridded();

// -- Methods

// -- Overridden methods

    virtual bool globalDomain() const;

// -- Class members
    // None

// -- Class methods

    static size_t computeN(double first, double last, double inc, const char *n_name, const char *first_name, const char *last_name);

  private:

// No copy allowed

    Gridded(const Gridded&);
    Gridded& operator=(const Gridded&);

// -- Members

// -- Methods



// -- Overridden methods
    virtual void setComplexPacking(grib_info&) const;
    virtual void setSimplePacking(grib_info&) const;
    virtual void setSecondOrderPacking(grib_info&) const;

    virtual void cropToDomain(const param::MIRParametrisation &parametrisation, data::MIRField&, util::MIRStatistics&) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const Gridded& p)
    //  { p.print(s); return s; }

};


}  // namespace repres
}  // namespace mir
#endif

