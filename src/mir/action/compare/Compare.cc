/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/compare/Compare.h"

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
#include "mir/input/GribFileInput.h"  // TODO better
#include "mir/input/MIRInput.h"


namespace mir {
namespace action {
namespace compare {


namespace {


static eckit::Mutex* local_mutex = 0;
static std::map< std::string, ComparisonFactory* > *m = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;


static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, ComparisonFactory* >();
}


}  // (anonymous namespace)


bool field_is_angle_degrees(const data::MIRField& field) {
    if (field.dimensions()==1) {
        const size_t id = field.paramId(0);
        return (id==140113) ||    // wefxd
               (id==140230);      // mwd
    }
    return false;
}


bool field_is_vector_cartedian2d(const data::MIRField& field) {
    if (field.dimensions()==2) {
        const size_t id1 = field.paramId(0), id2 = field.paramId(1);
        return (id1==131 && id2==132) ||    // u, v
               (id1==165 && id2==166);     // 10u, 10v
    }
    return false;
}


Compare::Compare(const param::MIRParametrisation& parametrisation) :
    action::Action(parametrisation) {
    optionsSetFrom(parametrisation);
}


size_t Compare::getNMaxDifferences(size_t N, const Compare::CompareOptions& options) {
    using util::compare::is_approx_one;
    using util::compare::is_approx_zero;
    const double compareCountFactor = std::max(0., std::min(1.,
        options.get< double >("compare.max_count_percent") * 0.01 ));
    return options.has("compare.max_count_diffs")? std::min(N, options.get< size_t >("compare.max_count_diffs"))
           : is_approx_one (compareCountFactor)? N
           : is_approx_zero(compareCountFactor)? 0
           : size_t(std::ceil(compareCountFactor * N));
}


bool Compare::compareResults(const CompareResults& a, const CompareResults& b, const CompareOptions& options) {
    bool cmp = (a==b);
    if (!cmp || options.get< bool >("compare.verbose")) {
        eckit::Log::info() << "\tfield A: " << a << "\n"
                              "\tfield B: " << b << std::endl;
    }
    return cmp;
}


void Compare::execute(context::Context& ctx) const {
    eckit::Log::info() << "Compare: options: " << options_ << std::endl;


    input::GribFileInput reference(options_.get< std::string >("compare.file"));

    input::MIRInput& input1 = ctx.input();
    input::MIRInput& input2(dynamic_cast< input::MIRInput& >( reference ));


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


    eckit::Log::info() << "Compare: input A " << (cmp? "==":"!=") << " input B." << std::endl;
}


bool Compare::sameAs(const action::Action& other) const {
    const Compare* o = dynamic_cast<const Compare*>(&other);
    return (o && options_ == o->options_);
}


void Compare::optionsReset() {
    param::SimpleParametrisation empty;
    optionsSetFrom(empty);
}


void Compare::optionsSetFrom(const param::MIRParametrisation& p) {
    options_.setFrom<std::string> (p, "compare.file",                    "");
    options_.setFrom<double>      (p, "compare.max_count_percent",       0);
    options_.setFrom<size_t>      (p, "compare.max_count_diffs",         0);
    options_.setFrom<size_t>      (p, "compare.max_count_fields",        1000);
    options_.setFrom<bool>        (p, "compare.compare_angle_in_polar",  true);
    options_.setFrom<bool>        (p, "compare.compare_vector_in_polar", true);
    options_.setFrom<bool>        (p, "compare.verbose",                 true);
}


void Compare::print(std::ostream& out) const {
    out << "Compare["
        <<  "options[" << options_ << "]"
        << "]";
}


ComparisonFactory::ComparisonFactory(const std::string& name) :
    name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if(m->find(name) != m->end()) {
        throw eckit::SeriousBug("ActionFactory: duplication action: " + name);
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


ComparisonFactory::~ComparisonFactory() {}


void ComparisonFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (std::map< std::string, ComparisonFactory* >::const_iterator j = m->begin() ; j != m->end() ; ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}


Compare* ComparisonFactory::build(const std::string& name, const param::MIRParametrisation& params) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    eckit::Log::debug<LibMir>() << "Looking for ActionFactory [" << name << "]" << std::endl;

    std::map< std::string, ComparisonFactory* >::const_iterator j = m->find(name);
    if (j == m->end()) {
        eckit::Log::error() << "No ComparisonFactory for [" << name << "]" << std::endl;
        eckit::Log::error() << "ComparisonFactories are:" << std::endl;
        for (j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No ComparisonFactory called ") + name);
    }

    return (*j).second->make(params);
}


}  // namespace compare
}  // namespace action
}  // namespace mir

