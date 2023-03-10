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
#include "mir/util/Exceptions.h"
#include "mir/util/IndexMapping.h"


namespace mir::action {


struct LL {
    double lat_;
    double lon_;
    LL(Latitude lat, Longitude lon) : lat_(lat.value()), lon_(lon.value()) {}
    bool operator<(const LL& other) const {
        // Order must be like natural scanning mode
        if (lat_ == other.lat_) {
            return lon_ < other.lon_;
        }

        return lat_ > other.lat_;
    }
};


Area::Area(const param::MIRParametrisation& parametrisation) : Action(parametrisation), caching_(LibMir::caching()) {
    ASSERT(key::Area::get(parametrisation_.userParametrisation(), bbox_));
    parametrisation_.get("caching", caching_);
}


Area::~Area() = default;


void Area::apply(const repres::Representation& repres, util::BoundingBox& bbox, util::IndexMapping& mapping) {
    std::map<LL, size_t> m;

    Latitude n  = 0;
    Latitude s  = 0;
    Longitude e = 0;
    Longitude w = 0;

    bool first = true;

    // Iterator is "unrotated", because the cropping area is expressed in before the rotation is applied
    for (const std::unique_ptr<repres::Iterator> it(repres.iterator()); it->next();) {
        const auto& point = it->pointUnrotated();

        // Log::debug() << point << " ====> " << bbox.contains(point) << std::endl;

        if (bbox.contains(point)) {
            const Latitude& lat = point.lat();
            const Longitude lon = point.lon().normalise(bbox.west());

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
    if (m.empty()) {
        std::ostringstream oss;
        oss << "Cropping " << repres << " to " << bbox << " returns no points";
        throw exception::UserError(oss.str());
    }

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
