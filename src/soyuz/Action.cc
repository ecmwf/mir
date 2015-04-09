// File Action.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "Action.h"


#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Once.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"

//-----------------------------------------------------------------------------


static eckit::Mutex *local_mutex = 0;
static std::map<std::string,ActionFactory*> *m = 0;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string,ActionFactory*>();
}

//-----------------------------------------------------------------------------

Action::Action(const MIRParametrisation& parametrisation):
    parametrisation_(parametrisation)
{
}

Action::~Action() {
}

//-----------------------------------------------------------------------------

ActionFactory::ActionFactory(const std::string& name):
        name_(name)
{

    pthread_once(&once,init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}

ActionFactory::~ActionFactory()
{
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);

}

Action* ActionFactory::build(const std::string& name, const MIRParametrisation& params)
{

    pthread_once(&once,init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, ActionFactory*>::const_iterator j = m->find(name);

    eckit::Log::info() << "Looking for ActionFactory [" << name << "]" << std::endl;

    if (j == m->end())
    {
        eckit::Log::error() << "No ActionFactory for [" << name << "]" << std::endl;
        eckit::Log::error() << "ActionFactories are:" << std::endl;
        for(j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No ActionFactory called ") + name);
    }

    return (*j).second->make(params);
}

