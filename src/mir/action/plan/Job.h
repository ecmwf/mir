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


#ifndef Job_H
#define Job_H

#include <string>

#include "eckit/memory/ScopedPtr.h"

#include "mir/param/SimpleParametrisation.h"

namespace mir {
namespace input {
class MIRInput;
}
namespace output {
class MIROutput;
}
namespace api {
class MIRJob;
}

namespace action {

class ActionPlan;

class Job  {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    Job(const api::MIRJob &job, input::MIRInput &input, output::MIROutput &output);

    // -- Destructor

    ~Job();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void execute() const;

    const ActionPlan& plan() const;

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



    // -- Overridden methods


    // -- Class members
    // None

    // -- Class methods
    // None

  private:


    // -- Members

    input::MIRInput &input_;
    output::MIROutput &output_;

    eckit::ScopedPtr<param::MIRParametrisation> combined_;
    eckit::ScopedPtr<action::ActionPlan> plan_;
    // -- Methods


    // -- Overridden methods

    // From MIRParametrisation


    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

};

}  // namespace action
}  // namespace mir
#endif

