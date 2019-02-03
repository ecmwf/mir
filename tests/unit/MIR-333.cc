/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <string>

#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"

#include "mir/config/LibMir.h"
#include "mir/method/MethodWeighted.h"
#include "mir/method/WeightMatrix.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace tests {
namespace unit {


CASE("MIR-333") {

    auto& log = eckit::Log::info();
    auto old = log.precision(16);


    param::DefaultParametrisation defaults;
    util::MIRStatistics stats;

    eckit::ScopedPtr<method::Method> method(method::MethodFactory::build("nn", defaults));
    auto nn = static_cast<method::MethodWeighted*>(method.get());
    ASSERT(nn);


    SECTION("NearestNeighbour: N640 > 1.5/1.5") {
        const eckit::PathName reference("MIR-333.001.mat");

        util::Increments grid(1.5, 1.5);
        repres::RepresentationHandle out(new repres::latlon::RegularLL(grid));
        repres::RepresentationHandle in(namedgrids::NamedGrid::lookup("N640").representation());

        method::WeightMatrix W(out->numberOfPoints(), in->numberOfPoints());
        nn->assemble(stats, W, *in, *out);

        ASSERT(W.rows() == W.nonZeros());

        if (!reference.exists()) {
            eckit::Log::warning() << "Saving reference '" << reference << "'" << std::endl;
            W.save(reference);
        }

        eckit::Log::debug<LibMir>() << "Loading reference '" << reference << "'" << std::endl;
        const method::WeightMatrix A(reference);
        const method::WeightMatrix& B(W);

        EXPECT(A.rows() == W.rows());
        EXPECT(A.cols() == W.cols());
        EXPECT(A.nonZeros() == W.nonZeros());

        for (auto a = A.begin(), b = B.begin(); a != A.end() && b != B.end(); ++a, ++b) {
            EXPECT(a.row() == b.row());
            EXPECT(a.col() == b.col());
            EXPECT(eckit::types::is_approximately_equal(*a, *b));
        }
    }


    log.precision(old);
}


} // namespace unit
} // namespace tests
} // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv, false);
}
