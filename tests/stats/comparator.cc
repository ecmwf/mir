// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/stats/Comparator.h"
#include "mir/util/Log.h"

#include "stats.h"


namespace mir::stats::tests {


std::unique_ptr<Comparator> build(const std::string& name, const param::MIRParametrisation& param1,
                                  const param::MIRParametrisation& param2) {
    return std::unique_ptr<Comparator>(ComparatorFactory::build(name, param1, param2));
}


/// Comparison (by name) of two fields, the reasons (empty if they compare equal)
std::string compare(const std::string& name, const data::MIRField& field1, const data::MIRField& field2,
                    const param::MIRParametrisation& param1 = param::SimpleParametrisation()) {
    // comparators keep references to the parametrisations
    param::SimpleParametrisation empty;
    auto comparator = build(name, param1, empty);
    auto reasons    = comparator->execute(field1, field2);

    Log::info() << *comparator << ": '" << reasons << "'" << std::endl;
    return reasons;
}


const std::string GRID = "{grid: [90, 90]}";


CASE("ComparatorT") {
    // a difference of 2 at the last point (at latitude -90)
    const auto field1 = make_field(GRID, {1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12.});
    const auto field2 = make_field(GRID, {1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 14.});

    const std::vector<std::string> names{"min-max", "scalar", "central-moments", "p-norms"};


    SECTION("identical fields") {
        for (const auto& name : names) {
            EXPECT_EQUAL(compare(name, field1, field1), "");
        }
    }


    SECTION("different fields (exact comparison by default)") {
        for (const auto& name : names) {
            EXPECT_EQUAL(compare(name, field1, field2), "\n* maximum difference (2) greater than 0");
        }
    }


    SECTION("absolute error") {
        param::SimpleParametrisation param;
        param.set("absolute-error", 2.);

        for (const auto& name : names) {
            EXPECT_EQUAL(compare(name, field1, field2, param), "");
        }

        param.set("absolute-error", 1.9);
        for (const auto& name : names) {
            EXPECT_EQUAL(compare(name, field1, field2, param), "\n* maximum difference (2) greater than 1.9");
        }
    }


    SECTION("statistics of the differences") {
        param::SimpleParametrisation param;
        param::SimpleParametrisation empty;

        auto minmax = build("min-max", param, empty);
        minmax->execute(field1, field2);
        EXPECT(contains(to_string(*minmax),
                        "Comparator[CounterBinary[count=12,max=2,maxIndex=11,maxIndexValue1=12,"
                        "maxIndexValue2=14,"));

        auto pnorms = build("p-norms", param, empty);
        pnorms->execute(field1, field2);
        EXPECT(contains(to_string(*pnorms), ",PNorms[L1=2,L2=2,Li=2]]"));

        auto scalar = build("scalar", param, empty);
        scalar->execute(field1, field2);
        EXPECT(contains(to_string(*scalar), ",Scalar[mean=0.166667,"));

        auto moments = build("central-moments", param, empty);
        moments->execute(field1, field2);
        EXPECT(contains(to_string(*moments), ",CentralMoments[mean=0.166667,"));

        // statistics are reset on each execution
        scalar->execute(field1, field1);
        EXPECT(contains(to_string(*scalar), ",Scalar[mean=0,stddev=0,"));
    }


    SECTION("missing values") {
        const auto missing1 = make_field(GRID, {MISSING, 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12.});
        const auto missing2 = make_field(GRID, {MISSING, 2., MISSING, 4., 5., 6., 7., 8., 9., 10., 11., 12.});

        EXPECT_EQUAL(compare("scalar", missing1, missing1), "");
        EXPECT_EQUAL(compare("scalar", missing1, missing2), "\n* different missing values (1) greater than 0");

        param::SimpleParametrisation param;
        param.set("ignore-different-missing-values", size_t(1));
        EXPECT_EQUAL(compare("scalar", missing1, missing2, param), "");
    }


    SECTION("latitude limits") {
        param::SimpleParametrisation param;

        param.set("ignore-below-latitude", -45.);
        EXPECT_EQUAL(compare("scalar", field1, field2, param), "");

        param::SimpleParametrisation above;
        above.set("ignore-above-latitude", 45.);
        EXPECT_EQUAL(compare("scalar", field1, field2, above), "\n* maximum difference (2) greater than 0");

        // same number of points (12), different representations
        const auto other = make_field("{grid: [120, 60]}", field2.values(0));
        EXPECT(contains(compare("scalar", field1, other, param),
                        "* cannot use latitude limits for different representations ("));
    }


    SECTION("values must be comparable") {
        const auto other = make_field("{grid: [10, 10]}", [](double, double) { return 0.; });
        EXPECT_THROWS_AS(compare("scalar", field1, other), eckit::AssertionFailed);
        EXPECT_THROWS_AS(compare("min-max", field1, other), eckit::AssertionFailed);
    }
}


CASE("ComparatorT (angles)") {
    SECTION("angle-degree") {
        const auto field1 = make_field(GRID, [](double, double) { return 350.; });
        const auto field2 = make_field(GRID, [](double, double) { return 10.; });

        EXPECT_EQUAL(compare("angle-degree", field1, field2), "\n* maximum difference (20) greater than 0");

        param::SimpleParametrisation param;
        param.set("absolute-error", 20.);
        EXPECT_EQUAL(compare("angle-degree", field1, field2, param), "");
    }


    SECTION("angle-radian") {
        const auto field1 = make_field(GRID, [](double, double) { return 0.1; });
        const auto field2 = make_field(GRID, [](double, double) { return 2. * M_PI - 0.1; });

        EXPECT(contains(compare("angle-radian", field1, field2), "\n* maximum difference (0.2) greater than 0"));

        param::SimpleParametrisation param;
        param.set("absolute-error", 0.21);
        EXPECT_EQUAL(compare("angle-radian", field1, field2, param), "");
    }
}


CASE("MissingValues") {
    const auto none   = make_field(GRID, [](double, double) { return 1.; });
    const auto north  = make_field(GRID, [](double lat, double) { return lat > 0 ? MISSING : 1.; });
    const auto north2 = make_field(GRID, [](double lat, double) { return lat > 0 ? MISSING : 2.; });

    // values are not compared, only where values are missing
    EXPECT_EQUAL(compare("missing-values", none, none), "");
    EXPECT_EQUAL(compare("missing-values", north, north2), "");

    auto reasons = compare("missing-values", north, none);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT(contains(reasons, "\n* " + std::to_string(i) + "\t"));
    }
    EXPECT_NOT(contains(reasons, "\n* 4\t"));
    EXPECT(contains(reasons, "\tmissing\t1.000000"));

    EXPECT(contains(compare("missing-values", none, north), "\t1.000000\tmissing"));

    const auto other = make_field("{grid: [120, 60]}", north.values(0));
    EXPECT_EQUAL(compare("missing-values", north, other), "\n* different representations, cannot compare");

    auto twoDimensions = north;
    auto values        = north.values(0);
    twoDimensions.update(values, 1, true);
    EXPECT_EQUAL(twoDimensions.dimensions(), 2);
    EXPECT_EQUAL(compare("missing-values", north, twoDimensions), "\n* different dimensions, cannot compare");

    param::SimpleParametrisation param;
    EXPECT_EQUAL(to_string(*build("missing-values", param, param)), "MissingValues[]");
}


CASE("Spectral") {
    const MIRValuesVector values1{2., 0., 3., 0., 0., 0., 1., 1., 0.5, 0., 2., 0.};
    auto values2 = values1;
    values2[0]   = 3.;

    const auto field1 = make_field("{grid: T2}", values1);
    const auto field2 = make_field("{grid: T2}", values2);

    // no limits, no comparison
    EXPECT_EQUAL(compare("spectral", field1, field2), "");

    param::SimpleParametrisation param;
    param.set("spectral-mean-difference-max", 0.5);
    param.set("spectral-energy-norm-difference-max", 0.5);
    EXPECT_EQUAL(compare("spectral", field1, field2, param), "\n* difference of spectral mean (1) greater than 0.5");

    param.set("spectral-mean-difference-max", 1.);
    param.set("spectral-energy-norm-difference-max", 0.4);
    EXPECT(contains(compare("spectral", field1, field2, param), "\n* difference of spectral energy norm (0.47") &&
           contains(compare("spectral", field1, field2, param), ") greater than 0.4"));

    param::SimpleParametrisation empty;
    auto comparator = build("spectral", empty, empty);
    EXPECT(contains(to_string(*comparator), "Spectral[meanDiff=nan,enormDiff=nan]"));

    comparator->execute(field1, field2);
    EXPECT(contains(to_string(*comparator), "Spectral[meanDiff=1,enormDiff=0.47"));
    EXPECT(contains(to_string(*comparator), ",Field1=Spectral[mean=2,"));
    EXPECT(contains(to_string(*comparator), ",Field2=Spectral[mean=3,"));
}


CASE("ComparatorFactory") {
    std::ostringstream list;
    ComparatorFactory::list(list);

    for (const auto& name : std::vector<std::string>{"angle-degree", "angle-radian", "central-moments", "min-max",
                                                     "missing-values", "p-norms", "scalar", "spectral"}) {
        EXPECT(contains(list.str(), name));
    }

    param::SimpleParametrisation param;
    EXPECT_THROWS_AS(ComparatorFactory::build("unknown", param, param), eckit::SeriousBug);
}


}  // namespace mir::stats::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
