// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>
#include <string>
#include <vector>


namespace eckit {
class JSON;
}

namespace mir {
namespace action {
class Action;
}
namespace context {
class Context;
}
namespace param {
class MIRParametrisation;
}
namespace repres {
class RepresentationHandle;
}
}  // namespace mir


namespace mir::action {


class ActionPlan : protected std::vector<Action*> {
private:
    // -- Types

    using container_t = std::vector<Action*>;
    using grids_t     = std::vector<std::unique_ptr<repres::RepresentationHandle>>;

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

    /// Execute (not dryrun), returning the grids (specs) of the input and after each action, as a JSON list
    std::string executeGridPlan(context::Context&) const;

    /// Actions (specs), as a JSON list
    void json(eckit::JSON&) const;
    std::string json_str() const;
    void dump(std::ostream&) const;
    void custom(std::ostream&) const;
    void compress();
    bool ended() const;

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
    const std::string dumpActionPlanFile_;
    const std::string dumpGridPlanFile_;

    // -- Methods

    /// Execute (not dryrun), recording the grids of the input and after each action (if asked)
    grids_t run(context::Context&, bool recordGrids) const;

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
