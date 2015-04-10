// File MIRLogic.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/logic/MIRLogic.h"
#include "soyuz/param/MIRParametrisation.h"
#include "soyuz/action/Action.h"


#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Once.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"


//-----------------------------------------------------------------------------


static eckit::Mutex *local_mutex = 0;
static std::map<std::string,MIRLogicFactory*> *m = 0;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string,MIRLogicFactory*>();
}

//-----------------------------------------------------------------------------

MIRLogic::MIRLogic(const MIRParametrisation &parametrisation):
    parametrisation_(parametrisation)
{
}

MIRLogic::~MIRLogic() {
}

void MIRLogic::add(std::vector<std::auto_ptr<Action> >& actions, const std::string& name) const {
    actions.push_back(std::auto_ptr<Action>(ActionFactory::build(name, parametrisation_)));
}

//-----------------------------------------------------------------------------

MIRLogicFactory::MIRLogicFactory(const std::string& name):
        name_(name)
{

    pthread_once(&once,init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}

MIRLogicFactory::~MIRLogicFactory()
{
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);

}

MIRLogic* MIRLogicFactory::build(const MIRParametrisation& params)
{

    std::string name;

    if(!params.get("logic", name)) {
        throw eckit::SeriousBug("MIRLogicFactory cannot get logic");
    }

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, MIRLogicFactory*>::const_iterator j = m->find(name);

    eckit::Log::info() << "Looking for MIRLogicFactory [" << name << "]" << std::endl;

    if (j == m->end())
    {
        eckit::Log::error() << "No MIRLogicFactory for [" << name << "]" << std::endl;
        eckit::Log::error() << "MIRLogicFactories are:" << std::endl;
        for(j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No MIRLogicFactory called ") + name);
    }

    return (*j).second->make(params);
}

