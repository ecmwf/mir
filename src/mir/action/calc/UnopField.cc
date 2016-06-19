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

#include "mir/action/calc/UnopField.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"

namespace mir {
namespace action {


template<class T>
UnopField<T>::UnopField(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
    ASSERT(parametrisation.get(T::param(), param_));
}

template<class T>
UnopField<T>::~UnopField() {
}

template<class T>
bool UnopField<T>::sameAs(const Action& other) const {
    const UnopField* o = dynamic_cast<const UnopField<T>*>(&other);
    return o ;
}

template<class T>
void UnopField<T>::print(std::ostream &out) const {
    out << "UnopField<" << T::name() << ">[param=" << param_ << "]";
}

template<class T>
void UnopField<T>::execute(data::MIRField & field, util::MIRStatistics& statistics) const {

    eckit::AutoTiming timing(statistics.timer_, statistics.calcTiming_);

    for (size_t j = 0; j < field.dimensions(); j++) {
        std::vector<double> &values = field.direct(j);

        size_t size = values.size();

        if (field.hasMissing()) {
            double missingValue = field.missingValue();

            for (size_t i = 0; i < size; i++) {
                if (values[i] != missingValue) {
                    values[i] = T::op(values[i]);
                }
            }

        } else {
            for (size_t i = 0; i < size; i++) {
                values[i] = T::op(values[i]);
            }
        }

        field.paramId(j, param_);

    }
}

struct round {
    static const char* name() { return "round.field"; }
    static const char* param() { return "round.field.param"; }

    typedef long long ll;

    static double op(double a) { return ll(a + 0.5); }
};

static ActionBuilder< UnopField<round> > _round(round::name());



}  // namespace action
}  // namespace mir

