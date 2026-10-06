// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>

#include "eckit/testing/Test.h"

#include "mir/caching/InMemoryCache.h"
#include "mir/caching/InMemoryCacheStatistics.h"
#include "mir/caching/InMemoryCacheUsage.h"


namespace mir::tests::unit {


using caching::InMemoryCacheUsage;
using Cache = caching::InMemoryCache<int>;


// Use an entry (inserting it if missing) in a cache user scope, like a MIR job does
void use(Cache& cache, const std::string& key) {
    caching::InMemoryCacheStatistics statistics;
    caching::InMemoryCacheUser<int> user(cache, statistics);

    if (cache.find(key) == cache.end()) {
        cache.insert(key, new int(0));
        cache.footprint(key, InMemoryCacheUsage(10, 0));
    }
}


bool cached(const Cache& cache, const std::string& key) {
    return cache.find(key) != cache.end();
}


CASE("InMemoryCache keeps entry larger than capacity, while hit") {
    Cache cache("test", 1, 1, "$MIR_TEST_CACHE_CAPACITY");

    use(cache, "a");
    EXPECT(cached(cache, "a"));

    use(cache, "a");
    EXPECT(cached(cache, "a"));
}


CASE("InMemoryCache purges entry larger than capacity, on miss") {
    Cache cache("test", 1, 1, "$MIR_TEST_CACHE_CAPACITY");

    use(cache, "a");
    use(cache, "b");
    EXPECT(!cached(cache, "a"));
    EXPECT(cached(cache, "b"));
}


CASE("InMemoryCache reserve purges entry larger than capacity") {
    Cache cache("test", 1, 1, "$MIR_TEST_CACHE_CAPACITY");

    use(cache, "a");
    cache.reserve(10, false);
    EXPECT(!cached(cache, "a"));
}


CASE("InMemoryCache reserve purges as many entries as needed") {
    Cache cache("test", 25, 25, "$MIR_TEST_CACHE_CAPACITY");

    use(cache, "a");
    use(cache, "b");

    cache.reserve(20, false);  // footprint + 20 - capacity = 15, more than one entry
    EXPECT(!cached(cache, "a"));
    EXPECT(!cached(cache, "b"));
}


CASE("InMemoryCache purges least recently used entry") {
    Cache cache("test", 15, 15, "$MIR_TEST_CACHE_CAPACITY");

    use(cache, "a");
    use(cache, "a");  // hit
    use(cache, "b");  // insert, more recent than the hit
    use(cache, "c");  // miss, purges one entry

    EXPECT(!cached(cache, "a"));
    EXPECT(cached(cache, "b"));
    EXPECT(cached(cache, "c"));
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
