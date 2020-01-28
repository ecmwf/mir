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

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/utils/Tokenizer.h"


namespace mir {
namespace compat {


static pthread_once_t once                          = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex                    = nullptr;
static std::map<std::string, GribCompatibility*>* m = nullptr;
static void init() {
    local_mutex = new eckit::Mutex();
    m           = new std::map<std::string, GribCompatibility*>();
}


GribCompatibility::GribCompatibility(const std::string& name) : name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


GribCompatibility::~GribCompatibility() {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name_) != m->end());
    m->erase(name_);
}


//=========================================================================


void GribCompatibility::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}

class CombinedGribCompatibility : public GribCompatibility {

    std::vector<const GribCompatibility*> list_;

    virtual void execute(const output::MIROutput& output, const param::MIRParametrisation& parametrisation,
                         grib_handle* h, grib_info& info) const {
        for (auto c : list_) {
            c->execute(output, parametrisation, h, info);
        }
    }

    virtual void printParametrisation(std::ostream& out, const param::MIRParametrisation& param) const {
        for (auto c : list_) {
            c->printParametrisation(out, param);
        }
    }

    virtual bool sameParametrisation(const param::MIRParametrisation& param1,
                                     const param::MIRParametrisation& param2) const {

        for (auto c : list_) {
            if (!c->sameParametrisation(param1, param2)) {
                return false;
            }
        }

        return true;
    }

    virtual void initialise(const metkit::MarsRequest& request, std::map<std::string, std::string>& postproc) const {
        for (auto c : list_) {
            c->initialise(request, postproc);
        }
    }

    virtual void print(std::ostream& out) const {
        out << "CombinedGribCompatibility[";
        const char* sep = "";
        for (auto c : list_) {
            out << sep << *c;
            sep = ",";
        }
        out << "]";
    }


public:
    CombinedGribCompatibility(const std::string& name, const std::vector<std::string>& names) :
        GribCompatibility(name) {
        for (auto& n : names) {
            list_.push_back(&GribCompatibility::lookup(n));
        }
    }
};

const GribCompatibility& GribCompatibility::lookup(const std::string& name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    auto j = m->find(name);
    if (j == m->end()) {

        static eckit::Tokenizer parse("/");
        std::vector<std::string> v;
        parse(name, v);

        if (v.size() > 1) {
            return *(new CombinedGribCompatibility(name, v));
        }

        list(eckit::Log::error() << "GribCompatibility: unknown '" << name << "', choices are: ");
        throw eckit::SeriousBug("GribCompatibility: unknown '" + name + "'");
    }

    return *(*j).second;
}


}  // namespace compat
}  // namespace mir
