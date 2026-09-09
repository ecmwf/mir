// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>

#include "mir/caching/legendre/LegendreLoader.h"
#include "mir/util/Atlas.h"
#include "mir/util/Types.h"


namespace mir::action::transform {


struct TransCache {

    using loader_t = caching::legendre::LegendreLoader;
    using cache_t  = atlas::trans::Cache;

    loader_t* loader_;
    cache_t transCache_;

    TransCache();

    TransCache(const TransCache&) = delete;
    TransCache(TransCache&&)      = delete;

    ~TransCache();

    void operator=(const TransCache&) = delete;
    void operator=(TransCache&&)      = delete;

    TransCache& operator=(cache_t&&);

    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& out, const TransCache& e) {
        e.print(out);
        return out;
    }
};


}  // namespace mir::action::transform
