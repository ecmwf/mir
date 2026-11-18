// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/filesystem/PathName.h"
#include "eckit/testing/Test.h"

#include "mir/config/LibMir.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/search/PointSearch.h"
#include "mir/search/Tree.h"
#include "mir/util/Log.h"

#include "interpolation.h"


namespace mir::interpolation::tests {


using search::PointSearch;
using search::Tree;
using search::TreeFactory;

using Neighbors = std::vector<PointSearch::PointValueType>;
using Indices   = std::set<size_t>;


PointXYZ point3(double lat, double lon) {
    return repres::Iterator::point_3D({lat, lon});
}


Indices payloads(const Neighbors& closest) {
    Indices idx;
    for (const auto& c : closest) {
        idx.insert(c.payload());
    }
    return idx;
}


CASE("point search (k-d trees)") {
    /*
     * 3x3 points, indexed (from the north-west):
     *   0 (20, 30)   1 (20, 40)   2 (20, 50)
     *   3 (10, 30)   4 (10, 40)   5 (10, 50)
     *   6 ( 0, 30)   7 ( 0, 40)   8 ( 0, 50)
     */
    auto r = representation("{grid: [10, 10], area: [20, 30, 0, 50]}");

    // (file-based trees are built if not found)
    const std::string relative = "mir/search/2/" + r->uniqueName() + ".kdtree";
    for (const auto& root : std::vector<std::string>{"/tmp", LibMir::cacheDir()}) {
        if (eckit::PathName path(root + "/" + relative); path.exists()) {
            path.unlink();
        }
    }

    for (const auto& trees : std::vector<std::string>{"memory", "mapped-temporary-file", "mapped-cache-file"}) {
        Log::info() << "point-search-trees: " << trees << std::endl;

        param::SimpleParametrisation param;
        param.set("point-search-trees", trees);

        // twice, the file-based trees are built then loaded
        for (size_t i = 0; i < 2; ++i) {
            const PointSearch search(param, *r);

            auto nearest = search.closestPoint(point3(11., 31.));
            EXPECT_EQUAL(nearest.payload(), 3);

            Neighbors closest;
            search.closestNPoints(point3(11., 31.), 1, closest);
            EXPECT(payloads(closest) == Indices({3}));

            search.closestNPoints(point3(11., 31.), 4, closest);
            EXPECT(payloads(closest) == Indices({0, 3, 4, 6}));
            EXPECT_EQUAL(closest.front().payload(), 3);

            search.closestWithinRadius(point3(10., 40.), 1.2e6, closest);
            EXPECT(payloads(closest) == Indices({1, 3, 4, 5, 7}));

            search.closestWithinRadius(point3(15., 35.), 1e5, closest);
            EXPECT(closest.empty());

            std::ostringstream str;
            str << search;
        }
    }
}


CASE("point search, the default trees") {
    auto r = representation("{grid: [10, 10], area: [10, 0, -10, 20]}");

    for (const auto& caching : std::vector<bool>{true, false}) {
        param::SimpleParametrisation param;
        param.set("caching", caching);

        const PointSearch search(param, *r);
        EXPECT_EQUAL(search.closestPoint(point3(-9., 19.)).payload(), 8);
    }
}


CASE("Tree, by default not implemented") {
    struct NoTree final : Tree {
        using Tree::Tree;
    };

    auto r = representation("{grid: [10, 10], area: [10, 0, -10, 20]}");
    NoTree tree(*r);

    EXPECT_EQUAL(tree.itemCount(), 9);

    std::vector<Tree::PointValueType> points;
    std::ostringstream str;

    EXPECT_THROWS_AS(tree.build(points), exception::SeriousBug);
    EXPECT_THROWS_AS(tree.insert({point3(0., 0.), 0}), exception::SeriousBug);
    EXPECT_THROWS_AS(tree.statsPrint(str, false), exception::SeriousBug);
    EXPECT_THROWS_AS(tree.statsReset(), exception::SeriousBug);
    EXPECT_THROWS_AS(tree.nearestNeighbour(point3(0., 0.)), exception::SeriousBug);
    EXPECT_THROWS_AS(tree.kNearestNeighbours(point3(0., 0.), 2), exception::SeriousBug);
    EXPECT_THROWS_AS(tree.findInSphere(point3(0., 0.), 1.), exception::SeriousBug);
    EXPECT_THROWS_AS(tree.ready(), exception::SeriousBug);
    EXPECT_THROWS_AS(tree.commit(), exception::SeriousBug);

    EXPECT_NO_THROW(tree.lock());
    EXPECT_NO_THROW(tree.unlock());

    str << tree;
    EXPECT_EQUAL(str.str(), "Tree[]\n");
}


CASE("TreeFactory") {
    auto r = representation("{grid: [10, 10], area: [10, 0, -10, 20]}");

    std::ostringstream list;
    TreeFactory::list(list);
    EXPECT(list.str().find("memory") != std::string::npos);
    EXPECT(list.str().find("mapped-cache-file") != std::string::npos);

    EXPECT_THROWS_AS(std::unique_ptr<Tree>(TreeFactory::build("unknown", *r)), exception::SeriousBug);

    std::unique_ptr<Tree> tree(TreeFactory::build("memory", *r));
    EXPECT(!tree->ready());

    std::ostringstream str;
    str << *tree;
    EXPECT_EQUAL(str.str(), "TreeMemory[]");
}


}  // namespace mir::interpolation::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
