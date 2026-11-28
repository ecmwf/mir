// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/area/Area.h"

#include <map>
#include <memory>
#include <sstream>
#include <variant>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/log/JSON.h"

#include "mir/config/LibMir.h"
#include "mir/key/Area.h"
#include "mir/param/MIRParametrisation.h"
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

    double n = 0.;
    double s = 0.;
    double e = 0.;
    double w = 0.;

    bool first = true;

    // points either projected (rotated) or of the grid frame (unrotated)
    const auto& points = projection ? repres.canonicalGrid() : repres.canonicalGridUnrotated();
    for (auto it = points.cbegin(); it; ++it) {
        const auto point = std::get<PointLonLat>(*it);

        if (bbox.contains(point)) {
            const auto lat = point.lat();
            const auto lon = util::normalise_longitude(point.lon(), bbox.west());

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


void Area::json(eckit::JSON& j) const {
    j << "area" << spec_value(bbox_);
}


bool Area::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Area*>(&other);
    return (o != nullptr) && (bbox_ == o->bbox_);
}


}  // namespace mir::action
