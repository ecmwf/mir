// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "eckit/testing/Test.h"

#include "mir/method/WeightMatrix.h"
#include "mir/util/Exceptions.h"


namespace mir::tests::unit {


CASE("WeightMatrix::validate") {
    SECTION("out-of-bounds") {
        const auto* when{"out-of-bounds"};
        const std::string what{
            "Invalid weight matrix (out-of-bounds): 1 row error, "
            "row 2: weights out-of-bounds, contents: (2,2,-0.1)"};

        method::WeightMatrix W(3, 3);
        W.setFromTriplets({{0, 0, 1.}, {1, 1, 0.}, {2, 2, -0.1}});

        try {
            W.validate(when, {false, true, false});
            ASSERT(false);
        }
        catch (exception::InvalidWeightMatrix& e) {
            EXPECT(e.what() == what);
        }
    }


    SECTION("weights sum") {
        const auto* when{"weights sum"};
        const std::string what{
            "Invalid weight matrix (weights sum): 2 row errors, "
            "row 1: weights sum not 0 or 1 (sum=0.5, 1-sum=0.5), contents: (1,1,0.5), "
            "row 2: weights sum not 0 or 1 (sum=0.1, 1-sum=0.9), contents: (2,2,0.1)"};

        method::WeightMatrix W(3, 3);
        W.setFromTriplets({{0, 0, 1.}, {1, 1, 0.5}, {2, 2, 0.1}});

        try {
            W.validate(when, {false, false, true});
            ASSERT(false);
        }
        catch (exception::InvalidWeightMatrix& e) {
            EXPECT(e.what() == what);
        }
    }


    SECTION("duplicate indices") {
        const auto* when{"duplicate indices"};
        const std::string what{
            "Invalid weight matrix (duplicate indices): 1 row error, "
            "row 1: duplicate indices, contents: (1,1,0.5), (1,1,0.5)"};

        method::WeightMatrix W(3, 3);
        W.setFromTriplets({{0, 0, 1.}, {1, 1, 0.5}, {1, 1, 0.5}});

        try {
            W.validate(when, {true, false, false});
            ASSERT(false);
        }
        catch (exception::InvalidWeightMatrix& e) {
            EXPECT(e.what() == what);
        }
    }


    SECTION("mixed") {
        const auto* when{"mixed"};
        const std::string what{
            "Invalid weight matrix (mixed): 2 row errors, "
            "row 0: duplicate indices, contents: (0,0,0.5), (0,0,0.5), "
            "row 1: weights sum not 0 or 1 (sum=0.5, 1-sum=0.5), contents: (1,1,0.5)"};

        method::WeightMatrix W(3, 3);
        W.setFromTriplets({{0, 0, 0.5}, {0, 0, 0.5}, {1, 1, 0.5}});

        try {
            W.validate(when, {true, false, true});
            ASSERT(false);
        }
        catch (exception::InvalidWeightMatrix& e) {
            EXPECT(e.what() == what);
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
