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

#include "mir/action/calc/BinopFields.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"

namespace mir {
namespace action {
template<class T>
BinopFields<T>::BinopFields(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
    ASSERT(parametrisation.get(T::param(), param_));
}

template<class T>
BinopFields<T>::~BinopFields() {
}

template<class T>
bool BinopFields<T>::sameAs(const Action& other) const {
    const BinopFields* o = dynamic_cast<const BinopFields<T>*>(&other);
    return o ;
}

template<class T>
void BinopFields<T>::print(std::ostream &out) const {
    out << "BinopFields<" << T::name() << ">[param=" << param_ << "]";
}

template<class T>
void BinopFields<T>::execute(data::MIRField & field, util::MIRStatistics& statistics) const {

    eckit::AutoTiming timing(statistics.timer_, statistics.calcTiming_);

    ASSERT(field.dimensions() == 2);

    std::vector<double> &values0 = field.direct(0);
    const std::vector<double> &values1 = field.values(1);

    size_t size = values0.size();
    ASSERT(values0.size() == values1.size());

    if (field.hasMissing()) {
        double missingValue = field.missingValue();

        for (size_t i = 0; i < size; i++) {
            if (values0[i] == missingValue || values1[i] == missingValue) {
                values0[i] = missingValue;
            } else {
                values0[i] = T::op(values0[i], values1[i]);
            }
        }

    } else {
        for (size_t i = 0; i < size; i++) {
            values0[i] = T::op(values0[i], values1[i]);
        }
    }

    field.dimensions(1);
    field.paramId(0, param_);
}

namespace {
struct add {
    static const char* name() { return "add.fields"; }
    static const char* param() { return "add.fields.param"; }

    static double op(double a, double b) { return a + b; }
};

static ActionBuilder< BinopFields<add> > _add(add::name());

struct sub {
    static const char* name() { return "sub.fields"; }
    static const char* param() { return "sub.fields.param"; }

    static double op(double a, double b) { return a - b; }
};

static ActionBuilder< BinopFields<sub> > _sub(sub::name());

}



}  // namespace action
}  // namespace mir

