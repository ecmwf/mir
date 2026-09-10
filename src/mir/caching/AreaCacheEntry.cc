// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/caching/AreaCacheEntry.h"

#include "eckit/filesystem/PathName.h"
#include "eckit/io/AutoCloser.h"
#include "eckit/serialisation/FileStream.h"

#include "mir/util/Log.h"
#include "mir/util/Trace.h"


namespace mir::caching {


void AreaCacheEntry::print(std::ostream& out) const {
    out << "AreaCacheEntry[size=" << mapping_.size() << ",bbox=" << bbox_
        << ",size=" << Log::Bytes(sizeof(util::IndexMapping::value_type) * mapping_.size()) << "]";
}


AreaCacheEntry::~AreaCacheEntry() = default;


size_t AreaCacheEntry::footprint() const {
    return sizeof(*this) + mapping_.capacity() * sizeof(size_t);
}


void AreaCacheEntry::save(const eckit::PathName& path) const {
    trace::Timer timer("Saving cropping to cache");

    eckit::FileStream f(path, "w");
    auto c = eckit::closer(f);

    f << bbox_.north();
    f << bbox_.west();
    f << bbox_.south();
    f << bbox_.east();

    f << mapping_.size();
    for (const auto& i : mapping_) {
        f << i;
    }
}


void AreaCacheEntry::load(const eckit::PathName& path) {
    trace::Timer timer("Loading cropping from cache");

    eckit::FileStream f(path, "r");
    auto c = eckit::closer(f);

    Latitude n;
    Longitude w;
    Latitude s;
    Longitude e;

    f >> n;
    f >> w;
    f >> s;
    f >> e;

    bbox_ = util::BoundingBox(n, w, s, e);

    size_t size;
    f >> size;

    mapping_.clear();
    mapping_.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        util::IndexMapping::value_type j;
        f >> j;
        mapping_.push_back(j);
    }
}


}  // namespace mir::caching
