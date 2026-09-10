// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <ostream>

#include "mir/util/BoundingBox.h"
#include "mir/util/IndexMapping.h"


namespace eckit {
class PathName;
}


namespace mir::caching {


struct AreaCacheEntry {

    ~AreaCacheEntry();

    util::IndexMapping mapping_;
    util::BoundingBox bbox_;

    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& out, const AreaCacheEntry& e) {
        e.print(out);
        return out;
    }

    size_t footprint() const;
    const util::BoundingBox& boundingBox() const { return bbox_; }

    void save(const eckit::PathName&) const;
    void load(const eckit::PathName&);
};


}  // namespace mir::caching
