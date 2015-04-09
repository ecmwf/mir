#include "GridSpec.h"

#include <eckit/Log.h>
#include <eckit/Exception.h>

static std::map<std::string,GridSpecFactory*> theMap;

GridSpecFactory::GridSpecFactory(const std::string& name)
{
    theMap[name] = this;
}

GridSpec* GridSpecFactory::build(const Params& params)
{
    std::string name = params.get("gridspec");

    std::map<std::string,GridSpecFactory*>::iterator j = theMap.find(name);

    if(j == theMap.end())
    {
        Log::error() << "GridSpec factories are:" << std::endl;
        for(j = theMap.begin() ; j != theMap.end() ; ++j)
            Log::error() << (*j).first << std::endl;
        throw eckit::SeriousBug(name + ": cannot find GridSpec factory.");
    }

    return (*j).second->make(params);
}
