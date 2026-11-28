// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/geo/Search.h"
#include "eckit/testing/Test.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"
#include "mir/util/Search.h"

#include "interpolation.h"


namespace mir::interpolation::tests {


using Neighbours = eckit::geo::Search::Neighbours;
using Indices    = std::set<size_t>;


Indices indices(const Neighbours& closest) {
    Indices idx;
    for (const auto& c : closest) {
        idx.insert(c.index);
    }
    return idx;
}


CASE("make_search: k-d trees of the storage configured") {
    /*
     * 3x3 points, indexed (from the north-west):
     *   0 (20, 30)   1 (20, 40)   2 (20, 50)
     *   3 (10, 30)   4 (10, 40)   5 (10, 50)
     *   6 ( 0, 30)   7 ( 0, 40)   8 ( 0, 50)
     */
    auto r = representation("{grid: [10, 10], area: [20, 30, 0, 50]}");

    for (const auto& trees : std::vector<std::string>{"memory", "mapped-file", "mapped-temporary-file",
                                                      "mapped-cache-file", "mapped-anonymous-memory"}) {
        Log::info() << "point-search-trees: " << trees << std::endl;

        param::SimpleParametrisation param;
        param.set("point-search-trees", trees);

        // twice, the file-based trees are built then loaded
        for (size_t i = 0; i < 2; ++i) {
            const auto search = util::make_search(param, *r);

            EXPECT_EQUAL(search->search_nn(PointLonLat{31., 11.}).index, 3);
            EXPECT(indices(search->search_knn(PointLonLat{31., 11.}, 1)) == Indices({3}));

            const auto closest = search->search_knn(PointLonLat{31., 11.}, 4);
            EXPECT(indices(closest) == Indices({0, 3, 4, 6}));
            EXPECT_EQUAL(closest.front().index, 3);

            EXPECT(indices(search->search_radius(PointLonLat{40., 10.}, 1.2e6)) == Indices({1, 3, 4, 5, 7}));
            EXPECT(search->search_radius(PointLonLat{35., 15.}, 1e5).empty());

            std::ostringstream str;
            str << *search;
        }
    }

    param::SimpleParametrisation param;
    param.set("point-search-trees", "unknown");
    EXPECT_THROWS(util::make_search(param, *r));
}


CASE("make_search: the default k-d trees") {
    auto r = representation("{grid: [10, 10], area: [10, 0, -10, 20]}");

    for (const auto& caching : std::vector<bool>{true, false}) {
        param::SimpleParametrisation param;
        param.set("caching", caching);

        const auto search = util::make_search(param, *r);
        EXPECT_EQUAL(search->search_nn(PointLonLat{19., -9.}).index, 8);
    }
}


CASE("make_search: indices of the values as held (Representation::canonicalGrid)") {
    // encoded from the south-east, held from the north-west
    auto r = representation("{grid: [10, 10], area: [10, 0, -10, 20], order: i-j+}");

    const param::SimpleParametrisation param;
    const auto search = util::make_search(param, *r);
    EXPECT_EQUAL(search->search_nn(PointLonLat{1., 9.}).index, 0);
    EXPECT_EQUAL(search->search_nn(PointLonLat{19., -9.}).index, 8);
}


CASE("search_loader: eckit::geo loaders, and former mir names") {
    EXPECT_EQUAL(util::search_loader("memory"), "memory");
    EXPECT_EQUAL(util::search_loader("mapped-file"), "mapped-file");
    EXPECT_EQUAL(util::search_loader("mapped-cache-file"), "mapped-file");
    EXPECT_EQUAL(util::search_loader("mapped-temporary-file"), "memory");
    EXPECT_EQUAL(util::search_loader("mapped-anonymous-memory"), "memory");
}


}  // namespace mir::interpolation::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
