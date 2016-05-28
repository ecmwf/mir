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

#include "mir/action/misc/CheckerBoard.h"

#include <iostream>

#include "eckit/memory/ScopedPtr.h"

#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace action {


CheckerBoard::CheckerBoard(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
}


CheckerBoard::~CheckerBoard() {
}


bool CheckerBoard::sameAs(const Action& other) const {
    return false;
}

void CheckerBoard::print(std::ostream &out) const {
    out << "CheckerBoard[]";
}


void CheckerBoard::execute(data::MIRField &field) const {

    repres::RepresentationHandle representation(field.representation());
    bool normalize = false;
    parametrisation_.get("0-1", normalize);

    std::vector<long> frequencies;
    if(!parametrisation_.get("frequencies", frequencies)) {
        frequencies.push_back(16);
        frequencies.push_back(8);
    }

    bool hasMissing = field.hasMissing();
    double missingValue = field.missingValue();

    for (size_t k = 0; k < field.dimensions(); k++) {
        std::vector<double> &values = field.direct(k);

        double minvalue = 0;
        double maxvalue = 0;

        size_t first   = 0;
        for (; first < values.size(); ++first) {
            if (!hasMissing || values[first] != missingValue) {
                minvalue = values[first];
                maxvalue = values[first];
                break;
            }
        }

        if (first == values.size()) {
            // Only missing values
            return;
        }

        for (size_t i = first; i < values.size(); ++i) {
            if (!hasMissing || values[i] != missingValue) {
                minvalue = std::min(minvalue, values[i]);
                maxvalue = std::max(maxvalue, values[i]);
            }
        }

        size_t we = frequencies[0];
        size_t ns = frequencies[1];

        double dwe = 360.0 / we;
        double dns = 180.0 / ns;

        if (normalize) {
            maxvalue = 1;
            minvalue = 0;
        }

        // Assumes iterator scans in the same order as the values
        eckit::ScopedPtr<repres::Iterator> iter(representation->rotatedIterator());
        double lat = 0;
        double lon = 0;

        std::vector<double> v;
        v.push_back(minvalue);
        v.push_back(maxvalue);

        std::map<std::pair<size_t, size_t>, size_t> boxes;

        size_t b = 0;
        for (size_t r = 0; r < we; r++) {
            for (size_t c = 0; c < ns; c++) {
                boxes[std::make_pair(r, c)] = b;
                b++;
                b %= v.size();
            }
            b++;
            b %= v.size();
        }


        size_t j = 0;

        while (iter->next(lat, lon)) {

            lat = 90 - lat;

            while (lon >= 369) {
                lon -= 360;
            }

            while (lon < 0) {
                lon += 360;
            }

            size_t c = size_t(lat / dns);
            size_t r = size_t(lon / dwe);

            if (!hasMissing || values[j] != missingValue) {
                values[j] = boxes[std::make_pair(r, c)];
            }

            j++;
        }

        ASSERT(j == values.size());
    }
}


namespace {
static ActionBuilder< CheckerBoard > action("misc.checkerboard");
}


}  // namespace action
}  // namespace mir

