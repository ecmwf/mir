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


#include "mir/action/filter/AdjustWindsAtPoles.h"

#include <limits>
#include <memory>
#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir::action {


bool AdjustWindsAtPoles::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const AdjustWindsAtPoles*>(&other);
    return (o != nullptr);
}


void AdjustWindsAtPoles::print(std::ostream& out) const {
    out << "AdjustWindsAtPoles[]";
}


void AdjustWindsAtPoles::execute(context::Context& ctx) const {
    auto& field = ctx.field();

    ASSERT(field.dimensions() % 2 == 0);
    ASSERT(field.dimensions() > 0);

    const auto missingValue = field.hasMissing() ? field.missingValue() : std::numeric_limits<double>::quiet_NaN();

    const auto N = field.values(0).size();
    ASSERT(N > 0);

    // find points at pole(s)
    const auto* representation(field.representation());
    ASSERT(representation);

    std::vector<size_t> north;
    std::vector<size_t> south;

    for (const std::unique_ptr<repres::Iterator> it(representation->iterator()); it->next();) {
        const auto lat = it->pointUnrotated().lat();
        if (lat == Latitude::NORTH_POLE) {
            north.push_back(it->index());
        }
        else if (lat == Latitude::SOUTH_POLE) {
            south.push_back(it->index());
        }
    }


    // set v to zero at pole(s)
    for (size_t i = 0; i < field.dimensions(); i += 2) {
        auto& u = field.direct(i);
        auto& v = field.direct(i + 1);
        ASSERT(u.size() == N);
        ASSERT(v.size() == N);

        for (const auto& indices : {north, south}) {
            for (const auto& idx : indices) {
                if (u[idx] == missingValue || v[idx] == missingValue) {
                    u[idx] = missingValue;
                    v[idx] = missingValue;
                }
                else {
                    v[idx] = 0;
                }
            }
        }
    }
}


const char* AdjustWindsAtPoles::name() const {
    return "AdjustWindsAtPoles";
}


static const ActionBuilder<AdjustWindsAtPoles> __action("filter.adjust-winds-at-poles");


}  // namespace mir::action
