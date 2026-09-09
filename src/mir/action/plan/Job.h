// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>


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
