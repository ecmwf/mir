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

#include "mir/action/calc/Binop.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"

namespace mir {
namespace action {


template<class T>
Binop<T>::Binop(const param::MIRParametrisation &parametrisation):
    Action(parametrisation),
    scalar_(0) {
    ASSERT(parametrisation.get(T::name(), scalar_));
}

template<class T>
Binop<T>::~Binop() {
}

template<class T>
bool Binop<T>::sameAs(const Action& other) const {
    const Binop* o = dynamic_cast<const Binop<T>*>(&other);
    return o && (scalar_ == o->scalar_);
}

template<class T>
void Binop<T>::print(std::ostream &out) const {
    out << "Binop[size=" << scalar_ << "]";
}

template<class T>
void Binop<T>::execute(data::MIRField & field, util::MIRStatistics& statistics) const {

    // eckit::AutoTiming timing(statistics.timer_, statistics.frameTiming_);

    double scalar = scalar_;

    for (size_t i = 0; i < field.dimensions(); i++ ) {

        double missingValue = field.missingValue();
        std::vector<double> &values = field.direct(i);

        for(std::vector<double>::iterator j = values.begin(); j != values.end(); ++j) {
            *j = T::op(*j, scalar);
        }
    }
}

struct multiply { static const char* name() { return "multiply"; }
static double op(double a, double b) { return a*b; }};

static ActionBuilder< Binop<multiply> > _multiply(multiply::name());



}  // namespace action
}  // namespace mir

