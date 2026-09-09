// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/transform/TransCache.h"

#include <ostream>


namespace mir::action::transform {


TransCache::TransCache() : loader_(nullptr) {}


TransCache::~TransCache() {
    delete loader_;
}


TransCache& TransCache::operator=(cache_t&& transCache) {

    // on assignment, loader is discarded
    delete loader_;
    loader_     = nullptr;
    transCache_ = transCache;

    return *this;
}


void TransCache::print(std::ostream& s) const {
    s << "TransCache[";
    if (loader_ != nullptr) {
        s << *loader_;
    }
    s << "]";
}


}  // namespace mir::action::transform
