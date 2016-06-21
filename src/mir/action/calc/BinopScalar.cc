/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015

#include "mir/action/calc/BinopScalar.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/action/context/Context.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"
#include "mir/data/MIRField.h"

namespace mir {
namespace action {


template<class T>
BinopScalar<T>::BinopScalar(const param::MIRParametrisation &parametrisation):
    Action(parametrisation),
    scalar_(0) {
    ASSERT(parametrisation.get(T::name(), scalar_));
    ASSERT(parametrisation.get(T::param(), param_));
}

template<class T>
BinopScalar<T>::~BinopScalar() {
}

template<class T>
bool BinopScalar<T>::sameAs(const Action& other) const {
    const BinopScalar* o = dynamic_cast<const BinopScalar<T>*>(&other);
    return o && (scalar_ == o->scalar_);
}

template<class T>
void BinopScalar<T>::print(std::ostream &out) const {
    out << "BinopScalar<" << T::name() << ">[scalar=" << scalar_ << ",param=" << param_ << "]";
}

template<class T>
void BinopScalar<T>::execute(context::Context & ctx) const {

eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().calcTiming_);
    data::MIRField& field = ctx.field();
    double scalar = scalar_;

    if (field.hasMissing()) {
        double missingValue = field.missingValue();
        for (size_t i = 0; i < field.dimensions(); i++ ) {

            std::vector<double> &values = field.direct(i);

            for (std::vector<double>::iterator j = values.begin(); j != values.end(); ++j) {
                if (*j != missingValue) {
                    *j = T::op(*j, scalar);
                }
            }
        }

    } else {
        for (size_t i = 0; i < field.dimensions(); i++ ) {

            std::vector<double> &values = field.direct(i);

            for (std::vector<double>::iterator j = values.begin(); j != values.end(); ++j) {
                *j = T::op(*j, scalar);
            }
        }
    }

    for (size_t i = 0; i < field.dimensions(); i++ ) {
        field.paramId(i, param_);
    }
}

namespace {

struct mul {
    static const char* name() { return "mul.scalar"; }
    static const char* param() { return "mul.scalar.param"; }

    static double op(double a, double b) { return a * b; }
};

static ActionBuilder< BinopScalar<mul> > _mul(mul::name());

struct div {
    static const char* name() { return "div.scalar"; }
    static const char* param() { return "div.scalar.param"; }

    static double op(double a, double b) { return a / b; }
};

static ActionBuilder< BinopScalar<div> > _div(div::name());

struct add {
    static const char* name() { return "add.scalar"; }
    static const char* param() { return "add.scalar.param"; }

    static double op(double a, double b) { return a + b; }
};

static ActionBuilder< BinopScalar<add> > _add(add::name());

struct sub {
    static const char* name() { return "sub.scalar"; }
    static const char* param() { return "sub.scalar.param"; }

    static double op(double a, double b) { return a - b; }
};

static ActionBuilder< BinopScalar<sub> > _sub(sub::name());


struct min {
    static const char* name() { return "min.scalar"; }
    static const char* param() { return "min.scalar.param"; }

    static double op(double a, double b) { return std::min(a, b); }
};

static ActionBuilder< BinopScalar<min> > _min(min::name());

struct max {
    static const char* name() { return "max.scalar"; }
    static const char* param() { return "max.scalar.param"; }

    static double op(double a, double b) { return std::max(a, b); }
};

static ActionBuilder< BinopScalar<max> > _max(max::name());


}

}  // namespace action
}  // namespace mir

