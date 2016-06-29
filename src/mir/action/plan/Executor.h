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


#ifndef Executor_H
#define Executor_H

#include <iosfwd>
#include <vector>


namespace mir {

namespace context {
class Context;
}


namespace util {
class MIRStatistics;
}


namespace api {
class MIRWatcher;
}

namespace action {

class ActionNode;

class Executor {
  public:

// -- Exceptions
    // None

// -- Contructors

    Executor();

// -- Destructor

    virtual ~Executor(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    virtual void execute(context::Context& ctx, const ActionNode& node) = 0;
    virtual void wait() = 0;

    //=====================================

 // -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members


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

    Executor(const Executor&);
    Executor& operator=(const Executor&);

// -- Members


// -- Methods
    // None

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Executor& p) {
        p.print(s);
        return s;
    }

};


}  // namespace action
}  // namespace mir
#endif

