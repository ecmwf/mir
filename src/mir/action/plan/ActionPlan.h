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

#include <string>
#include <vector>


namespace mir {
namespace action {
class Action;
}
namespace api {
class MIREstimation;
}
namespace context {
class Context;
}
namespace param {
class MIRParametrisation;
}
}  // namespace mir


namespace mir::action {


class ActionPlan : protected std::vector<Action*> {
private:
    // -- Types

    using container_t = std::vector<Action*>;

public:
    // -- Exceptions
    // None

    // -- Constructors

    ActionPlan(const param::MIRParametrisation&);

    ActionPlan(const ActionPlan&) = delete;
    ActionPlan(ActionPlan&&)      = delete;

    // -- Destructor

    ~ActionPlan();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const ActionPlan&) = delete;
    void operator=(ActionPlan&&)      = delete;

    // -- Methods

    void add(const std::string& name);
    void add(const std::string& name, const std::string&, long);
    void add(const std::string& name, const std::string&, const std::string&);
    void add(const std::string& name, const std::string&, const std::string&, const std::string&, long);
    void add(const std::string& name, const std::string&, const std::string&, const std::string&, const std::string&);

    void add(Action*);
    void add(const std::string& name, param::MIRParametrisation*);

    void execute(context::Context&) const;
    void dump(std::ostream&) const;
    void custom(std::ostream&) const;
    void compress();
    bool ended() const;

    void estimate(context::Context&, api::MIREstimation&) const;

    const Action& action(size_t) const;

    using container_t::back;
    using container_t::begin;
    using container_t::cbegin;
    using container_t::cend;
    using container_t::crbegin;
    using container_t::crend;
    using container_t::empty;
    using container_t::end;
    using container_t::front;
    using container_t::operator[];
    using container_t::rbegin;
    using container_t::rend;
    using container_t::size;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    const param::MIRParametrisation& parametrisation_;
    std::vector<param::MIRParametrisation*> runtimes_;

    // -- Methods

    void print(std::ostream&) const;
    Action& action(size_t);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    const std::string dumpPlanFile_;
    const std::string dumpStatisticsFile_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const ActionPlan& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::action
