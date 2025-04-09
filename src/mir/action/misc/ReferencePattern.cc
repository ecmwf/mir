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


#include "mir/action/misc/ReferencePattern.h"

#include <cmath>
#include <memory>
#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Angles.h"
#include "mir/util/Exceptions.h"


namespace mir::action {


bool ReferencePattern::sameAs(const Action& /*unused*/) const {
    return false;
}


void ReferencePattern::print(std::ostream& out) const {
    out << "ReferencePattern[]";
}


void ReferencePattern::execute(context::Context& ctx) const {
    data::MIRField& field = ctx.field();

    repres::RepresentationHandle representation(field.representation());
    bool normalize = false;
    parametrisation().get("0-1", normalize);

    std::vector<double> frequencies{6., 3.};
    parametrisation().get("frequencies", frequencies);

    bool hasMissing     = field.hasMissing();
    double missingValue = field.missingValue();

    for (size_t k = 0; k < field.dimensions(); k++) {
        MIRValuesVector& values = field.direct(k);

        double minvalue = 0;
        double maxvalue = 0;

        size_t first = 0;
        for (; first < values.size(); ++first) {
            if (!hasMissing || values[first] != missingValue) {
                minvalue = values[first];
                maxvalue = values[first];
                break;
            }
        }

        if (first == values.size()) {
            // Only missing values
            continue;
        }

        for (size_t i = first; i < values.size(); ++i) {
            if (!hasMissing || values[i] != missingValue) {
                minvalue = std::min(minvalue, values[i]);
                maxvalue = std::max(maxvalue, values[i]);
            }
        }

        if (normalize) {
            maxvalue = 1;
            minvalue = 0;
        }

        double median = (minvalue + maxvalue) / 2;
        double range  = maxvalue - minvalue;

        double f1 = frequencies[0] / 2.0;
        double f2 = frequencies[1];

        for (const std::unique_ptr<repres::Iterator> it(representation->iterator()); it->next();) {
            auto& v = values[it->index()];
            if (!hasMissing || v != missingValue) {
                const auto& p = it->pointUnrotated();
                v             = range * std::sin(f1 * util::degree_to_radian(p.lon().value())) *
                        std::cos(f2 * util::degree_to_radian(p.lat().value())) * 0.5 +
                    median;
            }
        }
    }
}


const char* ReferencePattern::name() const {
    return "ReferencePattern";
}


static const ActionBuilder<ReferencePattern> __action("misc.pattern");


}  // namespace mir::action
