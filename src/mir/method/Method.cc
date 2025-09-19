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


#include "mir/method/Method.h"

#include <map>
#include <regex>
#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/utils/MD5.h"
#include "eckit/utils/StringTools.h"

#include "mir/api/mir_config.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"
#include "mir/util/ValueMap.h"


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
    const static std::map<eckit::Hash::digest_t, std::string> KNOWN_METHODS{
        {"052f0588fc55cdccb2b07212eba810c5", "linear"},
        {"321490e25ba1e17d44118b3d167834bc", "bilinear"},
        {"c942366c09cd3b86530029d7c714bebf", "nearest-neighbour"},
        {"ef991e74a612a0ccca7bab42e88487b4", "grid-box-average"},
    };

    const auto method_str = [](const Method& m) {
        std::ostringstream s;
        eckit::JSON j(s);

        j.startObject();
        m.json(j);
        j.endObject();

        return s.str();
    }(*this);

    const auto d = (eckit::MD5() << method_str).digest();
    if (const auto i = KNOWN_METHODS.find(d); i != KNOWN_METHODS.end()) {
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
    // spec {type: x, b_c=y, ...} maps to (interpolation=value, b-c=y, ...)
    const std::string NAME("interpolation");

    param::SimpleParametrisation user;
    if (auto trimmed = eckit::StringTools::trim(str); !trimmed.empty() && trimmed[0] == '{') {
        util::ValueMap map(eckit::YAMLParser::decodeString(str));

        for (bool modified = true; modified;) {
            modified = false;

            for (const auto& [k, v] : map) {
                if (auto s = k.as<std::string>(), t = s == "type" ? NAME : std::regex_replace(s, std::regex("_"), "-");
                    t != s) {
                    map[t] = v;
                    map.erase(s);
                    modified = true;
                    break;
                }
            }
        }

        map.set(user);
    }
    else if (!str.empty()) {
        user.set(NAME, str);
    }

    std::string name;
    param::CombinedParametrisation param(user);
    ASSERT(static_cast<const param::MIRParametrisation&>(param).get(NAME, name));

    return build(name, param);
}


}  // namespace mir::method
