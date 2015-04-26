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


#ifndef RuntimeParametrisation_H
#define RuntimeParametrisation_H

#include <string>

#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace param {


class RuntimeParametrisation : public SimpleParametrisation {
  public:

// -- Exceptions
    // None

// -- Contructors

    RuntimeParametrisation();

// -- Destructor

    ~RuntimeParametrisation(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

// void set(const std::string& name, const char* value);
//     void set(const std::string& name, const std::string& value);
    // void set(const std::string& name, bool value);
    void set(const std::string& name, long value);
    // void set(const std::string& name, double value);
    // void set(const std::string& name, double v1, double v2);
    // void set(const std::string& name, double v1, double v2, double v3, double v4);
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

    // void print(ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    RuntimeParametrisation(const RuntimeParametrisation&);
    RuntimeParametrisation& operator=(const RuntimeParametrisation&);

// -- Members

// -- Methods
    // None

// -- Overridden methods

    // From MIRParametrisation
    virtual void print(std::ostream&) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const RuntimeParametrisation& p)
    //	{ p.print(s); return s; }

};


}  // namespace param
}  // namespace mir
#endif

