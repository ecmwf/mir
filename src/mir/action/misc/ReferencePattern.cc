// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/misc/ReferencePattern.h"

#include <cmath>
#include <memory>
#include <ostream>
#include <variant>

#include "eckit/geo/Grid.h"
#include "eckit/log/JSON.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Angles.h"
#include "mir/util/Exceptions.h"


namespace mir::action {


bool ReferencePattern::sameAs(const Action& /*unused*/) const {
    return false;
}


void ReferencePattern::json(eckit::JSON& j) const {
    // as used by execute
    if (bool normalize = false; parametrisation().get("0-1", normalize)) {
        j << "0-1" << normalize;
    }
    if (std::vector<double> frequencies; parametrisation().get("frequencies", frequencies)) {
        j << "frequencies" << frequencies;
    }
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

        const auto& points = representation->canonicalGridUnrotated();
        for (auto it = points.cbegin(); it; ++it) {
            auto& v = values[it.index()];
            if (!hasMissing || v != missingValue) {
                const auto p = std::get<PointLonLat>(*it);
                v            = range * std::sin(f1 * util::degree_to_radian(p.lon())) *
                        std::cos(f2 * util::degree_to_radian(p.lat())) * 0.5 +
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
