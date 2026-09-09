// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
    for (const auto* file : {"stream=wave,param=swh", "stream=wave,param=swh,domain=m"}) {
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
