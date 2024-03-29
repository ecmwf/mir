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


#include <cmath>
#include <functional>
#include <ostream>
#include <sstream>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/util/Exceptions.h"
#include "mir/util/FormulaBinop.h"
#include "mir/util/Function.h"
#include "mir/util/MIRStatistics.h"


namespace mir::util {


FormulaBinop::FormulaBinop(const param::MIRParametrisation& parametrisation, const std::string& name, Formula* arg1,
                           Formula* arg2) :
    FormulaFunction(parametrisation, name, arg1, arg2) {}


FormulaBinop::~FormulaBinop() = default;


void FormulaBinop::print(std::ostream& out) const {
    out << '(' << *args_[0] << ") " << function_ << " (" << *args_[1] << ')';
}


template <typename T>
class Unop : public Function {

    T op_;

    void print(std::ostream& s) const override { s << name_; }

    void field(context::Context& ctx, context::Context& ctx1) const {
        auto timing(ctx.statistics().calcTimer());

        auto& field  = ctx.field();
        auto& field1 = ctx1.field();

        field.dimensions(field1.dimensions());

        for (size_t d = 0; d < field1.dimensions(); ++d) {
            const MIRValuesVector& values1 = field1.values(d);

            size_t size = values1.size();

            MIRValuesVector values(size);

            if (field1.hasMissing()) {
                double missingValue1 = field1.missingValue();

                for (size_t i = 0; i < size; i++) {
                    if (values1[i] == missingValue1) {
                        values[i] = missingValue1;
                    }
                    else {
                        values[i] = op_(values1[i]);
                    }
                }

                field.update(values, d);
                field.hasMissing(true);
                field.missingValue(missingValue1);
            }
            else {
                for (size_t i = 0; i < size; i++) {
                    values[i] = op_(values1[i]);
                }
                field.update(values, d);
            }
        }
    }

    void scalar(context::Context& ctx, context::Context& ctx1) const { ctx.scalar(op_(ctx1.scalar())); }

    void execute(context::Context& ctx) const override {

        context::Context a = ctx.pop();

        if (a.isField()) {
            return field(ctx, a);
        }

        if (a.isScalar()) {
            return scalar(ctx, a);
        }

        std::ostringstream oss;
        oss << "Cannot compute " << *this << " with " << a;
        throw exception::SeriousBug(oss.str());
    }


public:
    Unop(const char* name) : Function(name) {}
};


template <typename T>
class Binop : public Function {

    T op_;

    void print(std::ostream& s) const override { s << name_; }

    void fieldField(context::Context& ctx, context::Context& ctx1, context::Context& ctx2) const {
        auto timing(ctx.statistics().calcTimer());

        auto& field  = ctx.field();
        auto& field1 = ctx1.field();
        auto& field2 = ctx2.field();

        ASSERT(field1.dimensions() == field2.dimensions());
        field.dimensions(field1.dimensions());

        for (size_t d = 0; d < field1.dimensions(); ++d) {
            const MIRValuesVector& values1 = field1.values(d);
            const MIRValuesVector& values2 = field2.values(d);

            size_t size = values1.size();
            ASSERT(values1.size() == values2.size());

            MIRValuesVector values(size);

            if (field1.hasMissing() || field2.hasMissing()) {
                double missingValue1 = field1.missingValue();
                double missingValue2 = field2.missingValue();

                for (size_t i = 0; i < size; i++) {
                    if (values1[i] == missingValue1 || values2[i] == missingValue2) {
                        values[i] = missingValue1;
                    }
                    else {
                        values[i] = op_(values1[i], values2[i]);
                    }
                }

                field.update(values, d);
                field.hasMissing(true);
                field.missingValue(missingValue1);
            }
            else {
                for (size_t i = 0; i < size; i++) {
                    values[i] = op_(values1[i], values2[i]);
                }
                field.update(values, d);
            }
        }
    }

    void scalarScalar(context::Context& ctx, context::Context& ctx1, context::Context& ctx2) const {
        ctx.scalar(op_(ctx1.scalar(), ctx2.scalar()));
    }

    void fieldScalar(context::Context& ctx, context::Context& ctx1, context::Context& ctx2) const {
        auto timing(ctx.statistics().calcTimer());

        auto& field    = ctx.field();
        auto& field1   = ctx1.field();
        double scalar2 = ctx2.scalar();

        field.dimensions(field1.dimensions());

        for (size_t d = 0; d < field1.dimensions(); ++d) {
            const MIRValuesVector& values1 = field1.values(d);

            size_t size = values1.size();

            MIRValuesVector values(size);

            if (field1.hasMissing()) {
                double missingValue1 = field1.missingValue();

                for (size_t i = 0; i < size; i++) {
                    if (values1[i] == missingValue1) {
                        values[i] = missingValue1;
                    }
                    else {
                        values[i] = op_(values1[i], scalar2);
                    }
                }

                field.update(values, d);
                field.hasMissing(true);
                field.missingValue(missingValue1);
            }
            else {
                for (size_t i = 0; i < size; i++) {
                    values[i] = op_(values1[i], scalar2);
                }
                field.update(values, d);
            }
        }
    }

    void scalarField(context::Context& ctx, context::Context& ctx1, context::Context& ctx2) const {
        auto timing(ctx.statistics().calcTimer());

        double scalar1 = ctx1.scalar();
        auto& field    = ctx.field();
        auto& field2   = ctx2.field();

        field.dimensions(field2.dimensions());

        for (size_t d = 0; d < field2.dimensions(); ++d) {
            const MIRValuesVector& values2 = field2.values(d);

            size_t size = values2.size();

            MIRValuesVector values(size);

            if (field2.hasMissing()) {
                double missingValue2 = field2.missingValue();

                for (size_t i = 0; i < size; i++) {
                    if (values2[i] == missingValue2) {
                        values[i] = missingValue2;
                    }
                    else {
                        values[i] = op_(scalar1, values2[i]);
                    }
                }

                field.update(values, d);
                field.hasMissing(true);
                field.missingValue(missingValue2);
            }
            else {
                for (size_t i = 0; i < size; i++) {
                    values[i] = op_(scalar1, values2[i]);
                }
                field.update(values, d);
            }
        }
    }

    void execute(context::Context& ctx) const override {
        // ASSERT(ctx.size() == 2);

        context::Context ctx2 = ctx.pop();
        context::Context ctx1 = ctx.pop();

        if (ctx1.isScalar() && ctx2.isScalar()) {
            return scalarScalar(ctx, ctx1, ctx2);
        }

        if (ctx1.isField() && ctx2.isField()) {
            return fieldField(ctx, ctx1, ctx2);
        }

        if (ctx1.isField() && ctx2.isScalar()) {
            return fieldScalar(ctx, ctx1, ctx2);
        }

        if (ctx1.isScalar() && ctx2.isField()) {
            return scalarField(ctx, ctx1, ctx2);
        }

        std::ostringstream oss;
        oss << "Cannot compute " << *this << " with " << ctx1 << " and " << ctx2;
        throw exception::SeriousBug(oss.str());
    }


public:
    Binop(const char* name) : Function(name) {}
};


struct round {
    double operator()(double x) const { return std::round(x); }
};


struct sqrt {
    double operator()(double x) const { return std::sqrt(x); }
};


struct sin {
    double operator()(double x) const { return std::sin(x); }
};


struct cos {
    double operator()(double x) const { return std::cos(x); }
};


struct tan {
    double operator()(double x) const { return std::tan(x); }
};


struct asin {
    double operator()(double x) const { return std::asin(x); }
};


struct acos {
    double operator()(double x) const { return std::acos(x); }
};


struct atan {
    double operator()(double x) const { return std::atan(x); }
};


struct log {
    double operator()(double x) const { return std::log(x); }
};


struct log2 {
    double operator()(double x) const { return std::log2(x); }
};


struct log10 {
    double operator()(double x) const { return std::log10(x); }
};


struct exp {
    double operator()(double x) const { return std::exp(x); }
};


struct abs {
    double operator()(double x) const { return std::abs(x); }
};


struct atan2 {
    double operator()(double x, double y) const { return std::atan2(x, y); }
};


struct min {
    double operator()(double x, double y) const { return std::min(x, y); }
};


struct max {
    double operator()(double x, double y) const { return std::max(x, y); }
};


struct pow {
    double operator()(double x, double y) const { return std::pow(x, y); }
};


static const Unop<std::negate<double> > negate("neg");
static const Unop<std::logical_not<double> > logical_not("not");

static const Unop<log2> _log2("log2");
static const Unop<round> _round("round");
static const Unop<sqrt> _sqrt("sqrt");
static const Unop<sin> _sin("sin");
static const Unop<cos> _cos("cos");
static const Unop<tan> _tan("tan");
static const Unop<asin> _asin("asin");
static const Unop<acos> _acos("acos");
static const Unop<atan> _atan("atan");
static const Unop<log> _log("log");
static const Unop<log10> _log10("log10");
static const Unop<exp> _exp("exp");
static const Unop<abs> _abs("abs");

static const Binop<std::plus<double> > plus("+");
static const Binop<std::minus<double> > minus("-");
static const Binop<std::multiplies<double> > multiplies("*");
static const Binop<std::divides<double> > divides("/");
// static Binop<std::modulus<double> > modulus1("%");
// static Binop<std::modulus<double> > modulus2("mod");
static const Binop<std::equal_to<double> > equal_to("=");
static const Binop<std::not_equal_to<double> > not_equal_to("!=");
static const Binop<std::greater<double> > greater(">");
static const Binop<std::less<double> > less("<");
static const Binop<std::greater_equal<double> > greater_equal(">=");
static const Binop<std::less_equal<double> > less_equal("<=");
static const Binop<std::logical_and<double> > logical_and_1("&&");
static const Binop<std::logical_or<double> > logical_or_1("||");
static const Binop<std::logical_and<double> > logical_and_2("and");
static const Binop<std::logical_or<double> > logical_or_2("or");

static const Binop<atan2> _atan2("atan2");
static const Binop<min> _min("min");
static const Binop<max> _max("max");
static const Binop<pow> _pow_1("^");
static const Binop<pow> _pow_2("pow");


}  // namespace mir::util
