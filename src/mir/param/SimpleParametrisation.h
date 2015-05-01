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


#ifndef SimpleParametrisation_H
#define SimpleParametrisation_H

#include "mir/param/MIRParametrisation.h"
#include <string>
#include <map>
#include <set>


namespace mir {
namespace param {


class Setting;
class DelayedParametrisation;

class SimpleParametrisation : public MIRParametrisation {
  public:

// -- Exceptions
    // None

// -- Contructors

    SimpleParametrisation();

// -- Destructor

    virtual ~SimpleParametrisation(); // Change to virtual if base class

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

    void set(const std::string& name, const char* value);
    void set(const std::string& name, const std::string& value);
    void set(const std::string& name, bool value);
    void set(const std::string& name, long value);
    void set(const std::string& name, double value);
    void set(const std::string& name, DelayedParametrisation* value);

    void set(const std::string& name, std::vector<long>& value);
    void set(const std::string& name, std::vector<double>& value);

    void clear(const std::string& name);


    bool matches(const param::MIRParametrisation& metadata) const;

    size_t size() const;

// -- Overridden methods
    virtual bool has(const std::string& name) const;

    virtual bool get(const std::string& name, std::string& value) const;
    virtual bool get(const std::string& name, bool& value) const;
    virtual bool get(const std::string& name, long& value) const;
    virtual bool get(const std::string& name, double& value) const;
    virtual bool get(const std::string& name, std::vector<long>& value) const;
    virtual bool get(const std::string& name, std::vector<double>& value) const;

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    SimpleParametrisation(const SimpleParametrisation&);
    SimpleParametrisation& operator=(const SimpleParametrisation&);

// -- Members

    std::map<std::string, Setting*> settings_;


// -- Methods
    // None

// -- Overridden methods
    // None



    template<class T>
    bool _get(const std::string& name, T& value) const;

    template<class T>
    void _set(const std::string& name, const T& value);



// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    // friend std::ostream& operator<<(std::ostream& s,const SimpleParametrisation& p) {
    //     p.print(s);
    //     return s;
    // }

};


}  // namespace param
}  // namespace mir
#endif

