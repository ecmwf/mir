/*
 * (C) Copyright 1996- ECMWF.
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


#ifndef ThreadExecutor_h
#define ThreadExecutor_h

#include <iosfwd>

#include "mir/action/plan/Executor.h"

namespace mir {

namespace data {
class Context;
}


namespace util {
class MIRStatistics;
}


namespace api {
class MIRWatcher;
}

namespace action {

class Action;

class ThreadExecutor : public Executor {
  public:

// -- Exceptions
    // None

// -- Contructors

    ThreadExecutor(const std::string& name);

// -- Destructor

    ~ThreadExecutor(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods


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

    void print(std::ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    ThreadExecutor(const ThreadExecutor&);
    ThreadExecutor& operator=(const ThreadExecutor&);

// -- Members

    // eckit::ThreadPool pool_;

// -- Methods
    // None

// -- Overridden methods
    // None
    virtual void execute(context::Context& ctx, const ActionNode& node) const;
    virtual void wait() const;
    virtual void parametrisation(const param::MIRParametrisation &parametrisation);

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends



};


}  // namespace action
}  // namespace mir
#endif

