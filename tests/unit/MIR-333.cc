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

#include "eckit/config/Resource.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"

#include "mir/action/context/Context.h"
#include "mir/method/MethodWeighted.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Types.h"


namespace mir::tests::unit {


CASE("MIR-333") {

    // run with '--new-reference' to generate new reference data
    static const bool newReference = eckit::Resource<bool>("--new-reference", false);

    static const param::SimpleParametrisation empty;
    const param::CombinedParametrisation defaults(empty);

    std::unique_ptr<method::Method> method(method::MethodFactory::build("nn", defaults));
    auto nn = dynamic_cast<method::MethodWeighted*>(method.get());
    ASSERT(nn);


    SECTION("NearestNeighbour: N640 > 1.5/1.5") {
        const eckit::PathName reference("MIR-333.001.mat");

        util::Increments grid(1.5, 1.5);
        repres::RepresentationHandle out(new repres::latlon::RegularLL(grid));
        repres::RepresentationHandle in(namedgrids::NamedGrid::lookup("N640").representation());

        context::Context ctx;
        const method::WeightMatrix& B = nn->getMatrix(ctx, *in, *out);

        if (newReference) {
            Log::info() << "Saving reference '" << reference << "'" << std::endl;
            B.save(reference);
        }

        Log::info() << "Loading reference '" << reference << "'" << std::endl;
        const method::WeightMatrix A(reference);

        EXPECT(A.rows() == B.rows());
        EXPECT(A.cols() == B.cols());
        EXPECT(A.nonZeros() == B.nonZeros());

        for (auto a = A.begin(), b = B.begin(); a != A.end() && b != B.end(); ++a, ++b) {
            EXPECT(a.row() == b.row());
            EXPECT(a.col() == b.col());
            EXPECT(eckit::types::is_approximately_equal(*a, *b));
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
