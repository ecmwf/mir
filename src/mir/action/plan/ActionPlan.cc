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


#include "ActionPlan.h"

#include <fstream>

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
    for (auto& p : *this) {
        delete p;
    }

    for (auto& p : runtimes_) {
        delete p;
    }
}



void ActionPlan::add(const std::string &name)  {
    push_back(ActionFactory::build(name, parametrisation_));
}


void ActionPlan::add(const std::string &name, const std::string &param, long value) {
    param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param, value);
    push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(const std::string &name, const std::string &param, const std::string& value)  {
    param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param, value);
    push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(const std::string &name, const std::string &param1,  const std::string &value1, const std::string &param2, long value2) {
    param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param1, value1);
    runtime->set(param2, value2);
    push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(const std::string &name, const std::string &param1,  const std::string &value1, const std::string &param2, const std::string &value2) {
    param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param1, value1);
    runtime->set(param2, value2);
    push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(Action *action)  {
    push_back(action);
}


void ActionPlan::add(const std::string &name, param::MIRParametrisation* runtime) {
    // param::RuntimeParametrisation *runtime = new param::RuntimeParametrisation(parametrisation_);
    ASSERT(runtime);
    runtimes_.push_back(runtime);
    push_back(ActionFactory::build(name, *runtime, false));
}


void ActionPlan::execute(context::Context & ctx) const {

    std::string dumpPlanFile;
    parametrisation_.get("dump-plan-file", dumpPlanFile);

    if (dumpPlanFile.size()) {

        if (dumpPlanFile == "-") {
            custom(std::cout);
            std::cout << std::endl;
        }
        else {
            std::ofstream out(dumpPlanFile, std::ios::app);
            custom(out);
            out << std::endl;
        }
    }

    bool dryrun = false;
    if (parametrisation_.get("dryrun", dryrun) && dryrun) {
        return;
    }

    const char* sep = "###################################################################################";

    for (const auto& p : *this) {
        eckit::Log::debug<LibMir>() << "Executing:"
                                    << "\n" << sep
                                    << "\n" << *p
                                    << "\n" << sep
                                    << std::endl;
        p->perform(ctx);
        eckit::Log::debug<LibMir>() << "Result:"
                                    << "\n" << sep
                                    << "\n" << ctx
                                    << "\n" << sep
                                    << std::endl;
    }
}


void ActionPlan::compress() {

    eckit::Log::debug<LibMir>() << "ActionPlan::compress ===>" << std::endl;

    bool more = true;
    while (more) {
        more = false;

        for (size_t i = 0; i < size() - 1; ++i) {
            std::ostringstream oldAction;
            oldAction << action(i);

            if (action(i).mergeWithNext(action(i + 1))) {

                eckit::Log::debug<LibMir>()
                        << "ActionPlan::compress: "
                                            << "\n   " << oldAction.str()
                                            << "\n + " << action(i + 1)
                                            << "\n = " << action(i)
                                            << std::endl;
                
                delete at(i + 1);
                erase(begin() + i + 1);

                more = true;
                break;
            }

            if (action(i).deleteWithNext(action(i + 1))) {

                eckit::Log::debug<LibMir>()
                        << "ActionPlan::compress: "
                        << "\n   " << oldAction.str()
                        << "\n + " << action(i + 1)
                        << "\n = " << action(i + 1)
                        << std::endl;

                delete at(i);
                erase(begin() + i);

                more = true;
                break;
            }
        }

    }
    eckit::Log::debug<LibMir>() << "ActionPlan::compress <===" << std::endl;

}


const Action& ActionPlan::action(size_t n) const {
    return *at(n);
}


Action& ActionPlan::action(size_t n) {
    return *at(n);
}


void ActionPlan::print(std::ostream &out) const {
    out << "ActionPlan[";
    const char *arrow = "";
    for (const auto& p : *this) {
        out << arrow << *p;
        arrow = " ==> ";
    }
    out << "]";
}

void ActionPlan::dump(std::ostream &out) const {
    for (const auto& p : *this) {
        out << "      ==> " << *p << std::endl;
    }
}


void ActionPlan::custom(std::ostream &out) const {
    const char *sep = "";
    for (const auto& p : *this) {
        out << sep;
        p->custom(out);
        sep = "|";
    }
}


}  // namespace action
}  // namespace mir

