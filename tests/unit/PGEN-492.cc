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

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Representation.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"


namespace mir::tests::unit {


CASE("PGEN-492") {
    for (const std::string& file : {"stream=wave,param=swh", "stream=wave,param=swh,domain=m"}) {
        std::unique_ptr<input::MIRInput> input(new input::GribFileInput(file));
        ASSERT(input->next());

        std::string gridType;
        input->parametrisation().get("gridType", gridType);
        ASSERT(gridType == "reduced_ll");

        repres::RepresentationHandle repres(input->field().representation());
        auto domain = repres->domain();

        EXPECT(domain.isPeriodicWestEast());
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
