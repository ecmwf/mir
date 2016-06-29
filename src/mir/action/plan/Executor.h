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
#include <string>


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

namespace param {
class MIRParametrisation;
}

namespace action {

class ActionNode;

class Executor {
public:

// -- Exceptions
    // None

// -- Contructors


// -- Destructor


// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    virtual void execute(context::Context& ctx, const ActionNode& node) const = 0;
    virtual void wait() const = 0;

    //=====================================

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods

    static const Executor &lookup(const param::MIRParametrisation &parametrisation);
    static void list(std::ostream &);

protected:

    Executor(const std::string &name);
    virtual ~Executor(); // Change to virtual if base class

// -- Members

        std::string name_;


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

