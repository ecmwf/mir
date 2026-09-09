// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/filter/NablaFilter.h"

#include <ostream>
#include <set>

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Mutex.h"


namespace mir::action {


NablaFilter::NablaFilter(const param::MIRParametrisation& param) :
    Action(param), meshGeneratorParams_(param), polesMissingValues_(false) {
    param.get("nabla-poles-missing-values", polesMissingValues_);
}


void NablaFilter::print(std::ostream& out) const {
    out << name() << "[meshGeneratorParameters=" << meshGeneratorParams_ << "]";
}


void NablaFilter::custom(std::ostream& out) const {
    out << name() << "[meshGeneratorParameters=...]";
}


static util::once_flag once;
static util::recursive_mutex* local_mutex = nullptr;
static std::set<std::string>* m           = nullptr;

static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::set<std::string>();
}


NablaFilterFactory::~NablaFilterFactory() = default;


NablaFilterFactory::NablaFilterFactory(const std::string& name) : ActionFactory("filter." + name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->insert(name);
}


void NablaFilterFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const auto* sep = "";
    for (const auto& j : *m) {
        out << sep << j;
        sep = ", ";
    }
}


}  // namespace mir::action
