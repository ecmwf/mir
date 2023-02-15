/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#pragma once

#include <memory>


namespace mir {
namespace action {
class ActionPlan;
}
namespace api {
class MIRJob;
class MIREstimation;
}  // namespace api
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
}  // namespace mir


namespace mir::action {


class Job {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Job(const api::MIRJob&, input::MIRInput&, output::MIROutput&, bool compress);

    // -- Destructor

    ~Job();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void execute(util::MIRStatistics&) const;
    void estimate(api::MIREstimation&) const;

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
    std::unique_ptr<const param::MIRParametrisation> combined_;
    std::unique_ptr<ActionPlan> plan_;

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


}  // namespace mir::action
