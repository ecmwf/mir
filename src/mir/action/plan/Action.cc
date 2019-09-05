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
/// @author Tiago Quintino
/// @date   Apr 2015


#include "eckit/exception/Exceptions.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "eckit/system/SystemInfo.h"

#include "mir/action/plan/Action.h"
#include "mir/config/LibMir.h"
#include "mir/util/BoundingBox.h"

// Includes needed for estimate() helper functions
#include "mir/action/context/Context.h"
#include "mir/api/MIREstimation.h"
#include "mir/data/MIRField.h"
#include "mir/method/Method.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"



namespace mir {
namespace action {


Action::Action(const param::MIRParametrisation &parametrisation):
    parametrisation_(parametrisation) {
}


Action::~Action() = default;


void Action::custom(std::ostream & out) const {
    out << *this;
}


void Action::perform(context::Context & ctx) const {
    eckit::TraceResourceUsage<LibMir> usage(name());
    execute(ctx);
}


bool Action::mergeWithNext(const Action&) {
    return false;
}


bool Action::deleteWithNext(const Action&) {
    return false;
}


bool Action::isEndAction() const {
    return false;
}


bool Action::isCropAction() const {
    return false;
}


bool Action::canCrop() const {
    return false;
}


util::BoundingBox Action::outputBoundingBox() const {
    NOTIMP;
}

void Action::estimate(context::Context&, api::MIREstimation& estimation) const {
    std::ostringstream oss;
    oss << "Action::estimate not implemented for " << *this;
    throw eckit::SeriousBug(oss.str());
}


void Action::estimateNumberOfGridPoints(context::Context& ctx, api::MIREstimation& estimation, const repres::Representation& out) {
    // eckit::Timer timer("estimateNumberOfGridPoints", std::cerr);
    estimation.numberOfGridPoints(ctx.field().representation()->numberOfPoints());
}


void Action::estimateMissingValues(context::Context& ctx, api::MIREstimation& estimation, const repres::Representation& out) {

    return;

    data::MIRField& field = ctx.field();
    ASSERT(field.dimensions() == 1);
    if (field.hasMissing()) {


        eckit::Timer timer("estimateMissingValues", std::cerr);

        param::DefaultParametrisation runtime;
        param::CombinedParametrisation combined(runtime, runtime, runtime);
        std::unique_ptr< method::Method > method(method::MethodFactory::build("nn", combined));

        util::MIRStatistics dummy; // TODO: use the global one
        context::Context ctx(field, dummy);
        method->execute(ctx, *field.representation(), out);


        size_t missing = 0;

        const MIRValuesVector& values = field.values(0);
        double missingValue = field.missingValue();

        for(size_t i = 0; i < values.size(); ++i) {
            if(values[i] == missingValue) {
                missing++;
            }
        }

        estimation.missingValues(missing);
    }
}


//----------------------------------------------------------------------------------------------------------------------


namespace {
static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex *local_mutex = nullptr;
static std::map<std::string, ActionFactory *> *m = nullptr;
static std::map<std::string, std::string> aliases;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, ActionFactory *>();
}
}  // (anonymous namespace)


ActionFactory::ActionFactory(const std::string &name):
    name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("ActionFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


ActionFactory::~ActionFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    m->erase(name_);
}


Action *ActionFactory::build(const std::string& name, const param::MIRParametrisation& params, bool exact) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "ActionFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {

        if (!exact) {
            // pass

            auto k = aliases.find(name);
            if (k != aliases.end()) {
                j = m->find((*k).second);
            } else {
                for (auto p = m->begin() ; p != m->end() ; ++p) {
                    if ((*p).first.find(name) != std::string::npos) {

                        if (j != m->end()) {
                            std::ostringstream oss;
                            oss << "ActionFactory: ambiguous '" << name << "'"
                                << ", could be '" << (*j).first  << "'"
                                << " or '" << (*p).first << "'";
                            eckit::Log::error() << "   " << (*j).first << std::endl;
                            throw eckit::SeriousBug(oss.str());
                        }

                        j = p;
                    }
                }

                if (j != m->end()) {
                    aliases[name] = (*j).first;
                }
            }
        }
        if (j == m->end()) {
            list(eckit::Log::error() << "ActionFactory: unknown '" << name << "', choices are: ");
            throw eckit::SeriousBug("ActionFactory: unknown '" + name + "'");
        }
    }

    return (*j).second->make(params);
}


void ActionFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}




}  // namespace action
}  // namespace mir

