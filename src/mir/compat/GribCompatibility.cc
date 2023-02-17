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


#include "mir/compat/GribCompatibility.h"

#include "eckit/utils/Tokenizer.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::compat {


static util::once_flag once;
static util::recursive_mutex* local_mutex           = nullptr;
static std::map<std::string, GribCompatibility*>* m = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, GribCompatibility*>();
}


GribCompatibility::GribCompatibility(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


GribCompatibility::~GribCompatibility() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    ASSERT(m->find(name_) != m->end());
    m->erase(name_);
}


void GribCompatibility::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


class CombinedGribCompatibility : public GribCompatibility {

    std::vector<const GribCompatibility*> list_;

    void execute(const output::MIROutput& output, const param::MIRParametrisation& parametrisation, grib_handle* h,
                 grib_info& info) const override {
        for (const auto* c : list_) {
            c->execute(output, parametrisation, h, info);
        }
    }

    void printParametrisation(std::ostream& out, const param::MIRParametrisation& param) const override {
        for (const auto* c : list_) {
            c->printParametrisation(out, param);
        }
    }

    bool sameParametrisation(const param::MIRParametrisation& param1,
                             const param::MIRParametrisation& param2) const override {

        for (const auto* c : list_) {
            if (!c->sameParametrisation(param1, param2)) {
                return false;
            }
        }

        return true;
    }

    void initialise(const metkit::mars::MarsRequest& request,
                    std::map<std::string, std::string>& postproc) const override {
        for (const auto* c : list_) {
            c->initialise(request, postproc);
        }
    }

    void print(std::ostream& out) const override {
        out << "CombinedGribCompatibility[";
        const char* sep = "";
        for (const auto* c : list_) {
            out << sep << *c;
            sep = ",";
        }
        out << "]";
    }

public:
    CombinedGribCompatibility(const std::string& name, const std::vector<std::string>& names) :
        GribCompatibility(name) {
        list_.reserve(names.size());
        for (const auto& n : names) {
            list_.push_back(&GribCompatibility::lookup(n));
        }
    }
};


const GribCompatibility& GribCompatibility::lookup(const std::string& name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    auto j = m->find(name);
    if (j == m->end()) {

        static eckit::Tokenizer parse("/");
        std::vector<std::string> v;
        parse(name, v);

        if (v.size() > 1) {
            return *(new CombinedGribCompatibility(name, v));
        }

        list(Log::error() << "GribCompatibility: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("GribCompatibility: unknown '" + name + "'");
    }

    return *(j->second);
}


}  // namespace mir::compat
