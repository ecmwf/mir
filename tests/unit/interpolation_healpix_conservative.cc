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

#include "eckit/testing/Test.h"

#include "mir/method/healpix/HEALPixConservative.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/proxy/HEALPix.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


namespace mir::tests::unit {


struct HEALPixConservative : method::healpix::HEALPixConservative {
    using method::healpix::HEALPixConservative::assemble;
    using method::healpix::HEALPixConservative::HEALPixConservative;
};


CASE("interpolation=healpix-conservative") {
    auto& log = Log::info();
    log.precision(16);


    // Setup

    std::unique_ptr<param::MIRParametrisation> param_A([]() {
        auto* custom = new param::SimpleParametrisation;
        custom->set("gridType", "healpix");
        custom->set("Nside", 2);
        custom->set("orderingConvention", "ring");
        custom->set("longitudeOfFirstGridPointInDegrees", 45.);
        return custom;
    }());

    std::unique_ptr<param::MIRParametrisation> param_B([]() {
        auto* custom = new param::SimpleParametrisation;
        custom->set("gridType", "healpix");
        custom->set("Nside", 3);
        custom->set("orderingConvention", "nested");
        custom->set("longitudeOfFirstGridPointInDegrees", 45.);
        return custom;
    }());

    repres::RepresentationHandle A(repres::RepresentationFactory::build(*param_A));
    ASSERT(A->numberOfPoints() == 12 * 2 * 2);

    repres::RepresentationHandle B(repres::RepresentationFactory::build(*param_B));
    ASSERT(B->numberOfPoints() == 12 * 3 * 3);

    std::unique_ptr<param::MIRParametrisation> param([]() {
        static const param::DefaultParametrisation defaults;
        static const param::SimpleParametrisation empty;
        return new param::CombinedParametrisation(empty, empty, defaults);
    }());

    HEALPixConservative interpol(*param);


    // Assemble interpolant matrix

    HEALPixConservative::WeightMatrix W;
    util::MIRStatistics stats;

    interpol.assemble(stats, W, *A, *B);

    EXPECT(W.rows() == B->numberOfPoints());
    EXPECT(W.cols() == A->numberOfPoints());

    // TODO more tests
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
