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


#ifndef MIRParametrisation_H
#define MIRParametrisation_H

#include <iosfwd>
#include <string>
#include <vector>


namespace mir {
namespace param {


class MIRParametrisation {
  public:

// -- Exceptions
    // None

// -- Contructors

    MIRParametrisation();

// -- Destructor

    virtual ~MIRParametrisation(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods
    // None

    virtual MIRParametrisation& set(const std::string& name, const char* value);
    virtual MIRParametrisation& set(const std::string& name, const std::string& value);
    virtual MIRParametrisation& set(const std::string& name, bool value);
    virtual MIRParametrisation& set(const std::string& name, long value);
    virtual MIRParametrisation& set(const std::string& name, double value);
    // virtual MIRParametrisation& set(const std::string& name, DelayedParametrisation* value);

    virtual MIRParametrisation& set(const std::string& name, const std::vector<long>& value);
    virtual MIRParametrisation& set(const std::string& name, const std::vector<double>& value);

    virtual MIRParametrisation& clear(const std::string& name);

    virtual bool has(const std::string& name) const = 0;
    virtual bool get(const std::string& name, std::string& value) const = 0;
    virtual bool get(const std::string& name, bool& value) const = 0;
    virtual bool get(const std::string& name, long& value) const = 0;
    virtual bool get(const std::string& name, double& value) const = 0;
    virtual bool get(const std::string& name, std::vector<long>& value) const = 0;
    virtual bool get(const std::string& name, std::vector<double>& value) const = 0;

    virtual bool get(const std::string& name, size_t& value) const;

    virtual void copyValuesTo(MIRParametrisation& other) const;

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

    virtual void print(std::ostream&) const = 0; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    MIRParametrisation(const MIRParametrisation&);
    MIRParametrisation& operator=(const MIRParametrisation&);

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

// -- Friends

    friend std::ostream& operator<<(std::ostream& s, const MIRParametrisation& p) {
        p.print(s);
        return s;
    }

};


}  // namespace param
}  // namespace mir
#endif

