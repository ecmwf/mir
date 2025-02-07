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


#include <map>
#include <sstream>

#include "mir/action/plan/Action.h"
#include "mir/api/MIREstimation.h"
#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"
#include "mir/util/Trace.h"


namespace mir::action {


Action::Action(const param::MIRParametrisation& param) : parametrisation_(param) {}


Action::~Action() = default;


void Action::custom(std::ostream& out) const {
    out << *this;
}


void Action::perform(context::Context& ctx) const {
    trace::ResourceUsage usage(name());
    execute(ctx);
}


bool Action::mergeWithNext(const Action& /*unused*/) {
    return false;
}


bool Action::deleteWithNext(const Action& /*unused*/) {
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

void Action::estimate(context::Context& /*unused*/, api::MIREstimation& /*estimation*/) const {
    std::ostringstream oss;
    oss << "Action::estimate not implemented for " << *this;
    throw exception::SeriousBug(oss.str());
}


void Action::estimateNumberOfGridPoints(context::Context& /*unused*/, api::MIREstimation& estimation,
                                        const repres::Representation& out) {
    // trace::Timer timer("estimateNumberOfGridPoints");
    estimation.numberOfGridPoints(out.numberOfPoints());
}


void Action::estimateMissingValues(context::Context& /*ctx*/, api::MIREstimation& /*estimation*/,
                                   const repres::Representation& /*out*/) {
#if 0
    data::MIRField& field = ctx.field();
    ASSERT(field.dimensions() == 1);
    if (field.hasMissing()) {
        trace::Timer timer("estimateMissingValues");

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
#endif
}


static util::once_flag once;
static util::recursive_mutex* local_mutex       = nullptr;
static std::map<std::string, ActionFactory*>* m = nullptr;
static std::map<std::string, std::string> aliases;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, ActionFactory*>();
}


ActionFactory::ActionFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("ActionFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


ActionFactory::~ActionFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


Action* ActionFactory::build(const std::string& name, const param::MIRParametrisation& params, bool exact) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> guard(*local_mutex);

    Log::debug() << "ActionFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {

        if (!exact) {
            // pass

            auto k = aliases.find(name);
            if (k != aliases.end()) {
                j = m->find(k->second);
            }
            else {
                for (auto p = m->begin(); p != m->end(); ++p) {
                    if (p->first.find(name) != std::string::npos) {

                        if (j != m->end()) {
                            std::ostringstream oss;
                            oss << "ActionFactory: ambiguous '" << name << "'"
                                << ", could be '" << j->first << "'"
                                << " or '" << p->first << "'";
                            Log::error() << "   " << j->first << std::endl;
                            throw exception::SeriousBug(oss.str());
                        }

                        j = p;
                    }
                }

                if (j != m->end()) {
                    aliases[name] = j->first;
                }
            }
        }
        if (j == m->end()) {
            list(Log::error() << "ActionFactory: unknown '" << name << "', choices are: ");
            throw exception::SeriousBug("ActionFactory: unknown '" + name + "'");
        }
    }

    return j->second->make(params);
}


void ActionFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> guard(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace mir::action
