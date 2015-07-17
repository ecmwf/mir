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


#ifndef JSONConfiguration_H
#define JSONConfiguration_H

#include <string>

#include "mir/param/MIRParametrisation.h"
#include "eckit/value/Value.h"

namespace eckit {
    class PathName;
};

namespace mir {
namespace param {


class JSONConfiguration : public MIRParametrisation {
  public:

// -- Exceptions
    // None

// -- Contructors

    JSONConfiguration(const eckit::PathName& path, char separator = '.');
    JSONConfiguration(std::istream&, char separator = '.');
    JSONConfiguration(const JSONConfiguration &other, const std::string &path);

    ~JSONConfiguration(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods
    // None

// -- Overridden methods

    virtual bool has(const std::string& name) const;

    virtual bool get(const std::string& name, std::string& value) const;
    virtual bool get(const std::string& name, bool& value) const;
    virtual bool get(const std::string& name, long& value) const;
    virtual bool get(const std::string& name, double& value) const;

    virtual bool get(const std::string& name, std::vector<long>& value) const;
    virtual bool get(const std::string& name, std::vector<double>& value) const;
    virtual bool get(const std::string& name, size_t& value) const;

// -- Class members
    // None

// -- Class methods


  protected:


// -- Destructor

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

    JSONConfiguration(const JSONConfiguration&);
    JSONConfiguration& operator=(const JSONConfiguration&);

// -- Members

    eckit::Value root_;
    char separator_;

// -- Methods

    eckit::Value lookUp(const std::string&, bool&) const;
    eckit::Value lookUp(const std::string&) const;

// -- Overridden methods

    // From MIRParametrisation
    virtual void print(std::ostream&) const;



// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const JSONConfiguration& p)
    //  { p.print(s); return s; }

};


}  // namespace param
}  // namespace mir
#endif

