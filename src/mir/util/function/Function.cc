/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/function/Function.h"

#include <cmath>
#include <map>
#include "eckit/memory/ScopedPtr.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace util {
namespace function {


double Constant::eval(const param::MIRParametrisation&) const {
    return value_;
}


double Variable::eval(const param::MIRParametrisation& p) const {
    double value;
    ASSERT(p.get(variable_, value));
    return value;
}


// --


static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;
static std::map<std::string, FunctionFactory*>* m_functions = 0;


static void init() {
    local_mutex = new eckit::Mutex();
    m_functions = new std::map<std::string, FunctionFactory* >();
}


FunctionFactory::FunctionFactory(const std::string& name) :
    name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m_functions->find(name) != m_functions->end()) {
        throw eckit::SeriousBug("FunctionFactory: duplicate '" + name + "'");
    }

    ASSERT(m_functions->find(name) == m_functions->end());
    (*m_functions)[name] = this;
}


FunctionFactory::~FunctionFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    if (m_functions) {
        m_functions->erase(name_);
    }
}


Function* FunctionFactory::build(const std::string& name, std::vector<Function*> args) {
    auto j = m_functions->find(name);
    if (j != m_functions->cend()) {
        return j->second->make(args);
    }

    list(eckit::Log::error() << "FunctionFactory: unknown '" << name << "', choices are: ");
    eckit::Log::error() << std::endl;
    throw eckit::SeriousBug("FunctionFactory: unknown '" + name + "'");
}


void FunctionFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m_functions) {
        out << sep << j.first;
        sep = ", ";
    }
}


// --


template<class T>
class BinaryOperatorFunction : public Function {
public:
    BinaryOperatorFunction(Function* arg1, Function* arg2) :
        arg1_(arg1),
        arg2_(arg2) {
        ASSERT(arg1_);
        ASSERT(arg2_);
    }
private:
    double eval(const param::MIRParametrisation& p) const {
        return operator_(arg1_->eval(p), arg2_->eval(p));
    }
    T operator_;
    eckit::ScopedPtr<Function> arg1_;
    eckit::ScopedPtr<Function> arg2_;
};


static BinaryOperationBuilder<BinaryOperatorFunction<std::plus<double> > >          __plus("+");
static BinaryOperationBuilder<BinaryOperatorFunction<std::minus<double> > >         __minus("-");
static BinaryOperationBuilder<BinaryOperatorFunction<std::multiplies<double> > >    __multiplies("*");
static BinaryOperationBuilder<BinaryOperatorFunction<std::divides<double> > >       __divides("/");
static BinaryOperationBuilder<BinaryOperatorFunction<std::equal_to<double> > >      __equal_to("=");
static BinaryOperationBuilder<BinaryOperatorFunction<std::not_equal_to<double> > >  __not_equal_to("!=");
static BinaryOperationBuilder<BinaryOperatorFunction<std::greater<double> > >       __greater(">");
static BinaryOperationBuilder<BinaryOperatorFunction<std::less<double> > >          __less("<");
static BinaryOperationBuilder<BinaryOperatorFunction<std::greater_equal<double> > > __greater_equal(">=");
static BinaryOperationBuilder<BinaryOperatorFunction<std::less_equal<double> > >    __less_equal("<=");
static BinaryOperationBuilder<BinaryOperatorFunction<std::logical_and<double> > >   __logical_and1("&&");
static BinaryOperationBuilder<BinaryOperatorFunction<std::logical_and<double> > >   __logical_and2("and");
static BinaryOperationBuilder<BinaryOperatorFunction<std::logical_or<double> > >    __logical_or1("||");
static BinaryOperationBuilder<BinaryOperatorFunction<std::logical_or<double> > >    __logical_or2("or");

struct min   { double operator()(double x, double y) const { return std::min(x, y); }};
struct max   { double operator()(double x, double y) const { return std::max(x, y); }};
struct atan2 { double operator()(double x, double y) const { return ::atan2(x, y); }};
struct pow   { double operator()(double x, double y) const { return ::pow(x, y); }};

static BinaryOperationBuilder<BinaryOperatorFunction<min> >   __min("min");
static BinaryOperationBuilder<BinaryOperatorFunction<max> >   __max("max");
static BinaryOperationBuilder<BinaryOperatorFunction<atan2> > __atan2("atan2");
static BinaryOperationBuilder<BinaryOperatorFunction<pow> >   __pow_1("^");
static BinaryOperationBuilder<BinaryOperatorFunction<pow> >   __pow_2("pow");


// --


template<class T>
class UnaryOperatorFunction : public Function {
public:
    UnaryOperatorFunction(Function* arg) :
        arg_(arg) {
        ASSERT(arg_);
    }
private:
    double eval(const param::MIRParametrisation& p) const {
        return operator_(arg_->eval(p));
    }
    T operator_;
    eckit::ScopedPtr<Function> arg_;
};


static UnaryOperationBuilder<UnaryOperatorFunction<std::negate<double> > >      __negate("neg");
static UnaryOperationBuilder<UnaryOperatorFunction<std::logical_not<double> > > __logical_not("not");

struct round { double operator()(double x) const { return ::round(x); }};
struct sqrt  { double operator()(double x) const { return ::sqrt(x); }};
struct sin   { double operator()(double x) const { return ::sin(x); }};
struct cos   { double operator()(double x) const { return ::cos(x); }};
struct tan   { double operator()(double x) const { return ::tan(x); }};
struct asin  { double operator()(double x) const { return ::asin(x); }};
struct acos  { double operator()(double x) const { return ::acos(x); }};
struct atan  { double operator()(double x) const { return ::atan(x); }};
struct log   { double operator()(double x) const { return ::log(x); }};
struct log2  { double operator()(double x) const { return ::log2(x); }};
struct log10 { double operator()(double x) const { return ::log10(x); }};
struct exp   { double operator()(double x) const { return ::exp(x); }};
struct abs   { double operator()(double x) const { return ::fabs(x); }};

static UnaryOperationBuilder<UnaryOperatorFunction<round> > __round("round");
static UnaryOperationBuilder<UnaryOperatorFunction<sqrt> >  __sqrt("sqrt");
static UnaryOperationBuilder<UnaryOperatorFunction<sin> >   __sin("sin");
static UnaryOperationBuilder<UnaryOperatorFunction<cos> >   __cos("cos");
static UnaryOperationBuilder<UnaryOperatorFunction<tan> >   __tan("tan");
static UnaryOperationBuilder<UnaryOperatorFunction<asin> >  __asin("asin");
static UnaryOperationBuilder<UnaryOperatorFunction<acos> >  __acos("acos");
static UnaryOperationBuilder<UnaryOperatorFunction<atan> >  __atan("atan");
static UnaryOperationBuilder<UnaryOperatorFunction<log> >   __log("log");
static UnaryOperationBuilder<UnaryOperatorFunction<log2> >  __log2("log2");
static UnaryOperationBuilder<UnaryOperatorFunction<log10> > __log10("log10");
static UnaryOperationBuilder<UnaryOperatorFunction<exp> >   __exp("exp");
static UnaryOperationBuilder<UnaryOperatorFunction<abs> >   __abs("abs");


}  // namespace function
}  // namespace util
}  // namespace mir
