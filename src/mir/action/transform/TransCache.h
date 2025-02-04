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
