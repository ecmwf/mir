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

#include "mir/action/plan/ActionPlan.h"
#include "mir/action/plan/Action.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/action/context/Context.h"
#include "mir/config/LibMir.h"

#include "eckit/exception/Exceptions.h"



namespace mir {
namespace action {

ActionPlan::ActionPlan(const param::MIRParametrisation &parametrisation):
    parametrisation_(parametrisation) {
}


ActionPlan::~ActionPlan() {
    for (std::vector<Action *>::iterator j = actions_.begin(); j != actions_.end(); ++j) {
        delete (*j);
    }

    for (std::vector<param::MIRParametrisation *>::iterator j = runtimes_.begin(); j != runtimes_.end(); ++j) {
        delete (*j);
    }
}

bool ActionPlan::empty() const {
    return actions_.empty();
}

void ActionPlan::add(const std::string &name)  {
    actions_.push_back(ActionFactory::build(name, parametrisation_));
}

void ActionPlan::add(Action *action)  {
    actions_.push_back(action);
}

size_t ActionPlan::size() const {
    return actions_.size();
}

const Action &ActionPlan::action(size_t n) const {
    ASSERT(n < actions_.size());
    return *actions_[n];
}

void ActionPlan::add(const std::string &name, const std::string &param, long value)  {
    param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);

    runtime->set(param, value);
    actions_.push_back(ActionFactory::build(name, *runtime));
}

void ActionPlan::add(const std::string &name, const std::string &param, const std::string& value)  {
    param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);

    runtime->set(param, value);
    actions_.push_back(ActionFactory::build(name, *runtime));
}

void ActionPlan::add(const std::string &name, const std::string &param, param::DelayedParametrisation *value) {
    param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);

    runtime->set(param, value);
    actions_.push_back(ActionFactory::build(name, *runtime));
}

void ActionPlan::add(const std::string &name, const std::string &param1,  const std::string& value1, const std::string &param2, long value2) {
    param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);

    runtime->set(param1, value1);
    runtime->set(param2, value2);

    actions_.push_back(ActionFactory::build(name, *runtime));
}

void ActionPlan::add(const std::string &name, const std::string &param1,  const std::string& value1, const std::string &param2, const std::string& value2) {
    param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);

    runtime->set(param1, value1);
    runtime->set(param2, value2);

    actions_.push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::execute(context::Context & ctx) const {
    for (std::vector<Action *>::const_iterator j = actions_.begin(); j != actions_.end(); ++j) {
        eckit::Log::debug<LibMir>() << "Executing " << **j << std::endl;
        (*j)->execute(ctx);
    }
}


void ActionPlan::print(std::ostream &out) const {
    out << "ActionPlan[";
    const char *arrow = "";
    for (std::vector<Action *>::const_iterator j = actions_.begin(); j != actions_.end(); ++j) {
        out << arrow << *(*j);
        arrow = " ==> ";
    }
    out << "]";
}

}  // namespace action
}  // namespace mir

