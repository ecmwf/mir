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


#include <memory>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"

// define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir::tests::unit {


CASE("distance-weighting") {
    using method::knn::distance::DistanceWeighting;
    using method::knn::distance::DistanceWeightingFactory;

    auto& log = Log::info();


    size_t ip = 0;
    const Point3 point1{0., 0., 0.};
    const Point3 point2{1., 0., 0.};
    const std::vector<search::PointSearch::PointValueType> neighbours{
        {{1., 0., 0.}, 1},
        {{0., 2., 0.}, 2},
        {{0., 0., 3.}, 3},
    };
    param::SimpleParametrisation param;
    param.set("distance", 2.);
    param.set("nclosest", 4);
    param.set("climate-filter-delta", 1.);
    param.set("distance-weighting-gaussian-stddev", 1.e6);


    SECTION("methods") {
        for (const auto* name : {
                 "climate-filter",
                 "cressman",
                 "gaussian",
                 "inverse-distance-weighting",
                 "inverse-distance-weighting-squared",
                 "nearest-neighbour",
                 "no",
                 "no-distance-weighting",
                 "pseudo-laplace",
                 "reciprocal",
                 "shepard",
             }) {
            log << "Test " << name << std::endl;
            std::vector<method::WeightMatrix::Triplet> triplets;
            std::unique_ptr<const DistanceWeighting> d(DistanceWeightingFactory::build(name, param));

            (*d)(ip, point1, neighbours, triplets);
            // EXPECT();

            (*d)(ip, point2, neighbours, triplets);
            // EXPECT();
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
