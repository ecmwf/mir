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


#ifndef mir_action_Job_h
#define mir_action_Job_h

#include <string>
#include "eckit/memory/ScopedPtr.h"


namespace mir {
namespace action {
class ActionPlan;
}
namespace api {
class MIRJob;
}
namespace input {
class MIRInput;
}
namespace output {
class MIROutput;
}
namespace param {
class MIRParametrisation;
}
namespace util {
class MIRStatistics;
}
}


namespace mir {
namespace action {


class Job  {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Job(const api::MIRJob&, input::MIRInput&, output::MIROutput&);

    // -- Destructor

    ~Job();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void execute(util::MIRStatistics& statistics) const;

    const ActionPlan& plan() const;

    const param::MIRParametrisation& parametrisation() const;

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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    input::MIRInput& input_;
    output::MIROutput& output_;

    eckit::ScopedPtr< const param::MIRParametrisation > combined_;
    eckit::ScopedPtr< action::ActionPlan > plan_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


}  // namespace action
}  // namespace mir


#endif

