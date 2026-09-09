// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <string>

#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tests::unit {


CASE("MIR-356") {
    std::unique_ptr<input::MIRInput> input(new input::GribFileInput("MIR-356.grib1"));
    ASSERT(input->next());

    auto& param = input->parametrisation();

    std::string gridType;
    param.get("gridType", gridType);
    ASSERT(gridType == "regular_gg");

    size_t N = 0;
    param.get("N", N);
    ASSERT(0 < N);

    size_t Ni = 0;
    param.get("Ni", Ni);
    ASSERT(0 < Ni && Ni != 4 * N);  // an unusual regular_gg

    size_t Nj = 0;
    param.get("Nj", Nj);
    ASSERT(0 < Nj);


    // Test: field 'validation' (on field access from input) compares iterator against field values size
    input->field();
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
