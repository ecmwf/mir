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


#include "mir/action/area/Area.h"

#include <map>
#include <memory>
#include <sstream>

#include "mir/config/LibMir.h"
#include "mir/key/Area.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Angles.h"
#include "mir/util/Exceptions.h"
#include "mir/util/IndexMapping.h"


namespace mir::action {


struct LL {
    double lat_;
    double lon_;
    LL(double lat, double lon) : lat_(lat), lon_(lon) {}
    bool operator<(const LL& other) const {
        // Order must be like natural scanning mode
        if (lat_ == other.lat_) {
            return lon_ < other.lon_;
        }

        return lat_ > other.lat_;
    }
};


Area::Area(const param::MIRParametrisation& param) : Action(param), caching_(LibMir::caching()) {
    ASSERT(key::Area::get(parametrisation().userParametrisation(), bbox_));
    parametrisation().get("caching", caching_);
}


void Area::apply(const repres::Representation& repres, util::BoundingBox& bbox, util::IndexMapping& mapping,
                 bool projection) {
    std::map<LL, size_t> m;

    double n = 0;
    double s = 0;
    double e = 0;
    double w = 0;

    const auto min_lon = bbox.west();

    bool first = true;

    // Point can be     interpreted "projected" or "non-projected"/"unrotated"
    for (const std::unique_ptr<repres::Iterator> it(repres.iterator()); it->next();) {
        const auto point(projection ? it->pointRotated() : it->pointUnrotated());

        // Log::debug() << point << " ====> " << bbox.contains(point) << std::endl;

        if (bbox.contains(point)) {
            const auto& lat = point.lat;
            const auto lon  = util::normalise_longitude(point.lon, min_lon);

            if (first) {
                n = s = lat;
                e = w = lon;
                first = false;
            }
            else {
                if (n < lat) {
                    n = lat;
                }
                if (s > lat) {
                    s = lat;
                }
                if (e < lon) {
                    e = lon;
                }
                if (w > lon) {
                    w = lon;
                }
            }

            // Make sure we don't visit duplicate points
            ASSERT(m.emplace(LL(lat, lon), it->index()).second);
        }
    }

    // Set mapping (don't support empty results)
    ASSERT_NONEMPTY_AREA("Area", !m.empty());

    mapping.clear();
    mapping.reserve(m.size());
    for (const auto& j : m) {
        mapping.push_back(j.second);
    }

    // Set resulting bounding box
    bbox = util::BoundingBox(n, w, s, e);
}


bool Area::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Area*>(&other);
    return (o != nullptr) && (bbox_ == o->bbox_);
}


}  // namespace mir::action
