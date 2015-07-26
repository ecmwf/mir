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

#include "mir/action/filter/AdjustWinds.h"

#include <iostream>
#include <cmath>

#include "eckit/exception/Exceptions.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"

namespace mir {
namespace action {


AdjustWinds::AdjustWinds(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {

    std::vector<double> value;
    ASSERT(parametrisation_.get("user.rotation", value));
    ASSERT(value.size() == 2);

    rotation_ = util::Rotation(value[0], value[1]);
}


AdjustWinds::~AdjustWinds() {
}

void AdjustWinds::print(std::ostream &out) const {
    out << "AdjustWinds[rotation=" << rotation_ << "]";
}

inline double radian(double x) { return x * M_PI / 180.0; }


void AdjustWinds::execute(data::MIRField &field) const {
    ASSERT((field.dimensions() % 2) == 0);

    std::vector<double> directions;
    directions.reserve(field.values(0).size());

    ASSERT(!field.hasMissing()); // For now
    field.representation()->windDirections(rotation_, directions);
    size_t size = directions.size();

    std::vector<double> c(size);
    std::vector<double> s(size);

    for (size_t i = 0; i < size; i++) {
        double d =  -radian(directions[i]);
        c[i] = cos(d);
        s[i] = sin(d);
        std::cout << directions[i] << " - " << c[i] << " - " << s[i] << std::endl;
    }

    for (size_t i = 0; i < field.dimensions(); i += 2 ) {

        // TODO: use matrix multiplication

        const std::vector<double> &u_values = field.values(i);
        const std::vector<double> &v_values = field.values(i + 1);

        ASSERT(u_values.size() == size);
        ASSERT(v_values.size() == size);

        std::vector<double> new_u_values(size);
        std::vector<double> new_v_values(size);

        for (size_t j = 0; j < size; j++) {
            new_u_values[j] = u_values[j] * c[j] - v_values[j] * s[j];
            new_v_values[j] = u_values[j] * s[j] + v_values[j] * c[j];
        }

        field.values(new_u_values, i);
        field.values(new_v_values, i + 1);
    }
}


namespace {
static ActionBuilder< AdjustWinds > filter("filter.adjust-winds");
}


}  // namespace action
}  // namespace mir

