// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/Method.h"

#include <map>
#include <memory>
#include <sstream>
#include <utility>

#include "eckit/log/JSON.h"

#include "mir/api/mir_config.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/param/SpecParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


extern "C" {
void omp_set_num_threads(int);
}


namespace mir::method {


Method::Method(const param::MIRParametrisation& params) : parametrisation_(params) {
    if constexpr (MIR_HAVE_OMP) {
        int num_threads = 1;
        if (params.get("parallel-omp-num-threads", num_threads)) {
            omp_set_num_threads(num_threads);
        }
    }
}


bool Method::getKnownName(std::string& name) const {
    // methods built from their name alone, by spec
    static const auto KNOWN = param::SpecParametrisation::knownSpecs<MethodFactory>(
        {"linear", "bilinear", "nearest-neighbour", "k-nearest", "grid-box-average", "grid-box-statistics",
         "healpix-resample", "healpix-statistics", "voronoi-statistics", "structured-bilinear", "structured-bicubic",
         "structured-biquasicubic"});

    if (const auto i = KNOWN.find(json_str(false)); i != KNOWN.end()) {
        name = i->second;
        return true;
    }

    return false;
}


void Method::json(eckit::JSON& j, bool lookupKnownMethods) const {
    if (lookupKnownMethods) {
        if (std::string name; getKnownName(name)) {
            j << name;
            return;
        }
    }

    j.startObject();
    json(j);
    j.endObject();
}


std::string Method::json_str(bool lookupKnownMethods) const {
    if (std::string name; lookupKnownMethods && getKnownName(name)) {
        return name;
    }

    std::ostringstream s;
    eckit::JSON j(s);
    json(j, false);
    return s.str();
}


Method::~Method() = default;


static util::once_flag once;
static util::recursive_mutex* local_mutex       = nullptr;
static std::map<std::string, MethodFactory*>* m = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, MethodFactory*>();
}


MethodFactory::MethodFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (auto it = m->find(name); it == m->end()) {
        (*m)[name] = this;
        return;
    }

    throw exception::SeriousBug("MethodFactory: duplicate '" + name + "'");
}


MethodFactory::~MethodFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


void MethodFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


Method* MethodFactory::build(const std::string& name, const param::MIRParametrisation& param) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (auto j = m->find(name); j != m->end()) {
        return j->second->make(param);
    }

    list(Log::error() << "MethodFactory: unknown '" << name << "', choices are: ");
    throw exception::SeriousBug("MethodFactory: unknown '" + name + "'");
}


Method* MethodFactory::make_from_string(const std::string& str) {
    // the method owns its parametrisation
    const std::string NAME("interpolation");
    std::unique_ptr<const param::MIRParametrisation> param(new param::SpecParametrisation(str, NAME));

    std::string name;
    ASSERT(param->get(NAME, name));

    auto* method  = build(name, *param);
    method->spec_ = std::move(param);
    return method;
}


}  // namespace mir::method
