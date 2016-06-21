/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   FormulaBinop.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#include <iostream>
#include <functional>
#include <cmath>


#include "mir/util/FormulaBinop.h"
#include "mir/util/Function.h"
#include "eckit/exception/Exceptions.h"
#include "mir/action/context/Context.h"


namespace mir {
namespace util {

//----------------------------------------------------------------------------------------------------------------------

FormulaBinop::FormulaBinop(const param::MIRParametrisation &parametrisation,
                           const std::string& name,
                           Formula* arg1,
                           Formula *arg2):
    FormulaFunction(parametrisation, name, arg1, arg2) {

}

FormulaBinop::~FormulaBinop() {
}

void FormulaBinop::print(std::ostream& out) const {
    out << '(' << *args_[0] << ") " << function_ << " (" << *args_[1] << ')' ;
}

//----------------------------------------------------------------------------------------------------------------------
template<typename T>
class Unop : public Function {

    T op_;

    virtual void print(std::ostream& s) const {
        s << name_;
    }

    virtual void execute(context::Context&  result,
                         const std::vector<context::Context*>& args) const {
        ASSERT(args.size() == 1);
        result.scalar(op_(args[0]->scalar()));
    }


public:
    Unop(const char* name) : Function(name) {}
};


template<typename T>
class Binop : public Function {

    T op_;

    virtual void print(std::ostream& s) const {
        s << name_;
    }

    virtual void execute(context::Context&  result,
                         const std::vector<context::Context*>& args) const {
        ASSERT(args.size() == 2);
        result.scalar(op_(args[0]->scalar(), args[1]->scalar()));
    }


public:
    Binop(const char* name) : Function(name) {}
};


static Binop<std::plus<double> > plus("+");
static Binop<std::minus<double> > minus("-");
static Binop<std::multiplies<double> > multiplies("*");
static Binop<std::divides<double> > divides("/");
// static Binop<std::modulus<double> > modulus1("%");
// static Binop<std::modulus<double> > modulus2("mod");

static Unop<std::negate<double> > negate("neg");


static Binop<std::equal_to<double> > equal_to("=");
static Binop<std::not_equal_to<double> > not_equal_to("!=");

static Binop<std::greater<double> > greater(">");
static Binop<std::less<double> > less("<");

static Binop<std::greater_equal<double> > greater_equal(">=");
static Binop<std::less_equal<double> > less_equal("<=");

static Binop<std::logical_and<double> > logical_and_1("&&");
static Binop<std::logical_or<double> > logical_or_1("||");

static Binop<std::logical_and<double> > logical_and_2("and");
static Binop<std::logical_or<double> > logical_or_2("or");


static Unop<std::logical_not<double> > logical_not("not");

//=========================================================
struct sqrt { double operator()(double x) const{ return ::sqrt(x); }};
static Unop<sqrt> _sqrt("sqrt");

struct sin { double operator()(double x) const{ return ::sin(x); }};
static Unop<sin> _sin("sin");

struct cos { double operator()(double x) const{ return ::cos(x); }};
static Unop<cos> _cos("cos");

//=========================================================

struct pow { double operator()(double x, double y) const{ return ::pow(x, y); }};
static Binop<pow> _pow_1("^");
static Binop<pow> _pow_2("pow");



} // namespace util
} // namespace mir
