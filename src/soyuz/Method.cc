// File Method.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "Method.h"
#include "MIRParametrisation.h"


#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Once.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"


//-----------------------------------------------------------------------------


static eckit::Mutex *local_mutex = 0;
static std::map<std::string, MethodFactory *> *m = 0;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, MethodFactory *>();
}

//-----------------------------------------------------------------------------

Method::Method() {
}

Method::~Method() {
}

//-----------------------------------------------------------------------------

MethodFactory::MethodFactory(const std::string &name):
    name_(name) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}

MethodFactory::~MethodFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);

}

Method *MethodFactory::build(const MIRParametrisation &params) {

    pthread_once(&once, init);


    std::string name;

    if (!params.get("method", name)) {
        throw eckit::SeriousBug("MethodFactory cannot get method");
    }

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, MethodFactory *>::const_iterator j = m->find(name);

    eckit::Log::info() << "Looking for MethodFactory [" << name << "]" << std::endl;

    if (j == m->end()) {
        eckit::Log::error() << "No MethodFactory for [" << name << "]" << std::endl;
        eckit::Log::error() << "MethodFactories are:" << std::endl;
        for (j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No MethodFactory called ") + name);
    }

    return (*j).second->make(params);
}


