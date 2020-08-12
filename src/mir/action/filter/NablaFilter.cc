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


#include "mir/action/filter/NablaFilter.h"

#include <ostream>
#include <set>

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {


NablaFilter::NablaFilter(const param::MIRParametrisation& param) : Action(param), meshGeneratorParams_("input", param) {
    param.get("nabla-poles-missing-values", polesMissingValues_ = false);
}


NablaFilter::~NablaFilter() = default;


void NablaFilter::print(std::ostream& out) const {
    out << name() << "[meshGeneratorParameters=" << meshGeneratorParams_ << "]";
}


void mir::action::NablaFilter::custom(std::ostream& out) const {
    out << name() << "[meshGeneratorParameters=...]";
}


static pthread_once_t once       = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = nullptr;
static std::set<std::string>* m  = nullptr;

static void init() {
    local_mutex = new eckit::Mutex();
    m           = new std::set<std::string>();
}


NablaFilterFactory::~NablaFilterFactory() = default;


NablaFilterFactory::NablaFilterFactory(const std::string& name) : ActionFactory("filter." + name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    m->insert(name);
}


void NablaFilterFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    auto sep = "";
    for (const auto& j : *m) {
        out << sep << j;
        sep = ", ";
    }
}


}  // namespace action
}  // namespace mir
