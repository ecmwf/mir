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

#include "mir/action/misc/ReferencePattern.h"

#include <iostream>
#include <cmath>

#include "eckit/memory/ScopedPtr.h"
#include "mir/action/context/Context.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/data/MIRField.h"

namespace mir {
namespace action {


ReferencePattern::ReferencePattern(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
}


ReferencePattern::~ReferencePattern() {
}

bool ReferencePattern::sameAs(const Action& other) const {
    return false;
}


void ReferencePattern::print(std::ostream &out) const {
    out << "ReferencePattern[]";
}


void ReferencePattern::execute(context::Context & ctx) const {
    data::MIRField& field = ctx.field();

    repres::RepresentationHandle representation(field.representation());
    bool normalize = false;
    parametrisation_.get("0-1", normalize);

    std::vector<long> frequencies;
    if(!parametrisation_.get("frequencies", frequencies)) {
        frequencies.push_back(6);
        frequencies.push_back(3);
    }

    bool hasMissing = field.hasMissing();
    double missingValue = field.missingValue();

    for (size_t k = 0; k < field.dimensions(); k++) {
        std::vector<double> &values = field.direct(k);

        double minvalue = 0;
        double maxvalue = 0;

        size_t first = 0;
        size_t count = 0;
        for (; first < values.size(); ++first) {
            if (!hasMissing || values[first] != missingValue) {
                minvalue = values[first];
                maxvalue = values[first];
                count++;
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
                count++;
            }
        }

        if (normalize) {
            maxvalue = 1;
            minvalue = 0;
        }

        double median = (minvalue + maxvalue) / 2;
        double range = maxvalue - minvalue;

        eckit::ScopedPtr<repres::Iterator> iter(representation->rotatedIterator());
        Latitude lat = 0;
        Longitude lon = 0;

        double f1 = frequencies[0] / 2.0;
        double f2 = frequencies[1];

        size_t j = 0;
        const double deg2rad = M_PI / 180.0;

        while (iter->next(lat, lon)) {

            if (!hasMissing || values[j] != missingValue) {
                values[j] = range * sin(f1 * lon.value() * deg2rad) * cos(f2 * lat.value() * deg2rad) * 0.5 + median;
            }

            j++;
        }

        ASSERT(j == values.size());

    }
}


namespace {
static ActionBuilder< ReferencePattern > action("misc.pattern");
}


}  // namespace action
}  // namespace mir

