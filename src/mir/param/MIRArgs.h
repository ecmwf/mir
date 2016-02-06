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


#ifndef MIRArgs_H
#define MIRArgs_H


#include "mir/param/SimpleParametrisation.h"

#include <set>
#include <vector>

namespace mir {
namespace param {
namespace option {
class Option;
}
}
}

namespace mir {
namespace param {

namespace option {
class Option;
}


class MIRArgs : public SimpleParametrisation {
  public:

    typedef void (*usage_proc)(const std::string& name);

// -- Exceptions
    // None

// -- Contructors

    MIRArgs(usage_proc usage, int args_count = -1);

    // Will take owneship of the content of the vector
    MIRArgs(usage_proc usage, int args_count, std::vector<option::Option*>& options);
    // TODO: more proper ArgOption

// -- Destructor

    ~MIRArgs(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    const std::set<std::string>& keys() const;
    const std::vector<std::string>& args() const;
    const std::string& args(size_t) const;


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

    MIRArgs(const MIRArgs&);
    MIRArgs& operator=(const MIRArgs&);

// -- Members

    std::set<std::string> keys_;
    std::vector<std::string> args_;
    std::vector<option::Option*> options_;

// -- Methods

    void init(usage_proc usage, int args_count);

// -- Overridden methods

    // From MIRParametrisation
    virtual void print(std::ostream&) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const MIRArgs& p)
    //  { p.print(s); return s; }

};


}  // namespace param
}  // namespace mir
#endif

