/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/statistics/Statistics.h"

#include <map>
#include <ostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/log/Plural.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/action/context/Context.h"
#include "mir/config/LibMir.h"
#include "mir/input/MIRInput.h"


namespace mir {
namespace action {
namespace statistics {


namespace {


static eckit::Mutex* local_mutex = 0;
static std::map< std::string, StatisticsFactory* > *m = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;


static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, StatisticsFactory* >();
}


}  // (anonymous namespace)


Statistics::Statistics(const param::MIRParametrisation& parametrisation) :
    action::Action(parametrisation) {
}


void Statistics::execute(context::Context& ctx) const {
#if 0
    input::MIRInput& input1 = ctx.input();


    bool next1 = input1.next();
    bool next2 = input2.next();
    size_t count1 = next1? 1 : 0;
    size_t count2 = next2? 1 : 0;

    bool cmp = true;
    while (cmp && next1 && next2) {

        // perform comparison
        cmp = compare(
                    input1.field(), input1.parametrisation(),
                    input2.field(), input2.parametrisation() );

        next1 = input1.next();
        next2 = input2.next();
        if (next1) { ++count1; }
        if (next2) { ++count2; }
    }
    cmp = cmp && (count1==count2);

    const size_t countMax = options_.get<size_t>("compare.max_count_fields");
    while (input1.next() && (count1<countMax)) { ++count1; }
    while (input2.next() && (count2<countMax)) { ++count2; }


    if (count1!=count2 || options_.get< bool >("compare.verbose")) {
        eckit::Log::info() << "\tinput A: " <<  eckit::Plural(count1, "field") << (count1>=countMax? " (possibly more)":"") << "\n"
                              "\tinput B: " <<  eckit::Plural(count2, "field") << (count2>=countMax? " (possibly more)":"") << std::endl;
    }


    eckit::Log::info() << "Statistics: input A " << (cmp? "==":"!=") << " input B." << std::endl;
#endif
}


bool Statistics::sameAs(const action::Action& other) const {
#if 0
    const Statistics* o = dynamic_cast<const Statistics*>(&other);
    return (o && options_ == o->options_);
#endif
    return false;
}


void Statistics::print(std::ostream& out) const {
    out << "Statistics["
//      <<  "options[" << options_ << "]"
        << "]";
}


StatisticsFactory::StatisticsFactory(const std::string& name) :
    name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if(m->find(name) != m->end()) {
        throw eckit::SeriousBug("StatisticsFactory: duplication action: " + name);
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


StatisticsFactory::~StatisticsFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);
}


void StatisticsFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (std::map< std::string, StatisticsFactory* >::const_iterator j = m->begin() ; j != m->end() ; ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}


Statistics* StatisticsFactory::build(const std::string& name, const param::MIRParametrisation& params) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    eckit::Log::debug<LibMir>() << "Looking for ActionFactory [" << name << "]" << std::endl;

    std::map< std::string, StatisticsFactory* >::const_iterator j = m->find(name);
    if (j == m->end()) {
        eckit::Log::error() << "No StatisticsFactory for [" << name << "]" << std::endl;
        eckit::Log::error() << "StatisticsFactories are:" << std::endl;
        for (j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No StatisticsFactory called ") + name);
    }

    return (*j).second->make(params);
}


}  // namespace statistics
}  // namespace action
}  // namespace mir

