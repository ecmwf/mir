// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <map>
#include <memory>
#include <sstream>
#include <utility>

#include "eckit/log/JSON.h"

#include "mir/action/plan/Action.h"
#include "mir/param/SpecParametrisation.h"
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


bool Action::isRegridAction() const {
    return false;
}


bool Action::canCrop() const {
    return false;
}


bool Action::getGriddedTargetName(std::string&) const {
    return false;
}


util::BoundingBox Action::outputBoundingBox() const {
    NOTIMP;
}


void Action::json(eckit::JSON&) const {
    throw exception::UserError(std::string("Action: spec not supported by ") + name());
}


bool Action::getKnownName(std::string& name) const {
    // actions built from their name alone, by spec
    static const auto KNOWN = param::SpecParametrisation::knownSpecs<ActionFactory>(
        {"filter.scalar-gradient", "filter.scalar-laplacian", "filter.uv-gradient", "filter.uv-divergence",
         "filter.uv-vorticity"});

    if (const auto i = KNOWN.find(json_str(false)); i != KNOWN.end()) {
        name = i->second;
        return true;
    }

    return false;
}


void Action::json(eckit::JSON& j, bool lookupKnownActions) const {
    if (std::string name; lookupKnownActions && getKnownName(name)) {
        j << name;
        return;
    }

    j.startObject();
    json(j);
    j.endObject();
}


std::string Action::json_str(bool lookupKnownActions) const {
    if (std::string name; lookupKnownActions && getKnownName(name)) {
        return name;
    }

    std::ostringstream s;
    eckit::JSON j(s);
    json(j, false);
    return s.str();
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


Action* ActionFactory::make_from_string(const std::string& str) {
    // the action owns its parametrisation
    const std::string NAME("action");
    std::unique_ptr<const param::MIRParametrisation> param(new param::SpecParametrisation(str, NAME));

    std::string name;
    if (!param->get(NAME, name)) {
        throw exception::UserError("ActionFactory: spec 'type' expected, from '" + str + "'");
    }

    auto* action  = build(name, *param);
    action->spec_ = std::move(param);
    return action;
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
