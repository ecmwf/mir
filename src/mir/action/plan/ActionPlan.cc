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

#include "eckit/exception/Exceptions.h"
#include "mir/action/plan/Action.h"
#include "mir/config/LibMir.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/action/context/Context.h"


namespace mir {
namespace action {


ActionPlan::ActionPlan(const param::MIRParametrisation &parametrisation):
    parametrisation_(parametrisation) {
}


ActionPlan::~ActionPlan() {
    for (auto& p : actions_) {
        delete p;
    }

    for (auto& p : runtimes_) {
        delete p;
    }
}


void ActionPlan::add(const std::string &name)  {
    actions_.push_back(ActionFactory::build(name, parametrisation_));
}


void ActionPlan::add(const std::string &name, const std::string &param, long value) {
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


void ActionPlan::add(const std::string &name, const std::string &param1,  const std::string &value1, const std::string &param2, long value2) {
    param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param1, value1);
    runtime->set(param2, value2);
    actions_.push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(const std::string &name, const std::string &param1,  const std::string &value1, const std::string &param2, const std::string &value2) {
    param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param1, value1);
    runtime->set(param2, value2);
    actions_.push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(const std::string &name, const std::string &param, param::DelayedParametrisation *value) {
    param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param, value);
    actions_.push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(const std::string &name, const std::string &param1, param::DelayedParametrisation *value1, const std::string &param2, long value2) {
    param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param1, value1);
    runtime->set(param2, value2);
    actions_.push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(const std::string &name, const std::string &param1, param::DelayedParametrisation *value1, const std::string &param2, const std::string &value2) {
    param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param1, value1);
    runtime->set(param2, value2);
    actions_.push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(Action *action)  {
    actions_.push_back(action);
}


void ActionPlan::add(const std::string &name, param::MIRParametrisation* runtime) {
    // param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    ASSERT(runtime);
    runtimes_.push_back(runtime);
    actions_.push_back(ActionFactory::build(name, *runtime, false));
}


void ActionPlan::execute(context::Context & ctx) const {

    const char* sep = "###################################################################################";

    for (const auto& p : actions_) {
        eckit::Log::debug<LibMir>() << "Executing:"
                                    << "\n" << sep
                                    << "\n" << *p
                                    << "\n" << sep
                                    << std::endl;
        p->execute(ctx);
        eckit::Log::debug<LibMir>() << "Result:"
                                    << "\n" << sep
                                    << "\n" << ctx
                                    << "\n" << sep
                                    << std::endl;
    }
}


bool ActionPlan::empty() const {
    return actions_.empty();
}


size_t ActionPlan::size() const {
    return actions_.size();
}


const Action &ActionPlan::action(size_t n) const {
    ASSERT(n < actions_.size());
    return *actions_[n];
}


void ActionPlan::print(std::ostream &out) const {
    out << "ActionPlan[";
    const char *arrow = "";
    for (const auto& p : actions_) {
        out << arrow << *p;
        arrow = " ==> ";
    }
    out << "]";
}

void ActionPlan::dump(std::ostream &out) const {
    for (const auto& p : actions_) {
        out << "      ==> " << *p << std::endl;
    }
}

}  // namespace action
}  // namespace mir

