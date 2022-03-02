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


#include "mir/action/misc/CheckerBoard.h"

#include <algorithm>
#include <memory>
#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace action {


CheckerBoard::CheckerBoard(const param::MIRParametrisation& parametrisation) : Action(parametrisation) {}


CheckerBoard::~CheckerBoard() = default;


bool CheckerBoard::sameAs(const Action& /*unused*/) const {
    return false;
}

void CheckerBoard::print(std::ostream& out) const {
    out << "CheckerBoard[]";
}


void CheckerBoard::execute(context::Context& ctx) const {
    data::MIRField& field = ctx.field();

    repres::RepresentationHandle representation(field.representation());
    bool normalize = false;
    parametrisation_.get("0-1", normalize);

    std::vector<size_t> frequencies{16, 8};
    parametrisation_.get("frequencies", frequencies);

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
            return;
        }

        for (size_t i = first; i < values.size(); ++i) {
            if (!hasMissing || values[i] != missingValue) {
                minvalue = std::min(minvalue, values[i]);
                maxvalue = std::max(maxvalue, values[i]);
            }
        }

        auto we = Longitude::GLOBE.value() / double(frequencies[0]);
        auto ns = Latitude::GLOBE.value() / double(frequencies[1]);

        if (normalize) {
            maxvalue = 1;
            minvalue = 0;
        }

        std::vector<double> v;
        v.push_back(minvalue);
        v.push_back(maxvalue);

        std::map<std::pair<size_t, size_t>, size_t> boxes;

        size_t b = 0;
        for (size_t r = 0; r < frequencies[0]; r++) {
            for (size_t c = 0; c < frequencies[1]; c++) {
                boxes[std::make_pair(r, c)] = b;
                b++;
                b %= v.size();
            }
            b++;
            b %= v.size();
        }

        for (const std::unique_ptr<repres::Iterator> it(representation->iterator()); it->next();) {
            auto& v = values.at(it->index());
            if (!hasMissing || v != missingValue) {
                const auto& p = it->pointUnrotated();

                Latitude lat  = Latitude::NORTH_POLE - p.lat();
                Longitude lon = p.lon().normalise(Longitude::GREENWICH);

                auto c = size_t(lat.value() / ns);
                auto r = size_t(lon.value() / we);

                v = double(boxes[std::make_pair(r, c)]);
            }
        }
    }
}


const char* CheckerBoard::name() const {
    return "CheckerBoard";
}


static const ActionBuilder<CheckerBoard> __action("misc.checkerboard");


}  // namespace action
}  // namespace mir
