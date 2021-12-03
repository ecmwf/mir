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
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Log.h"


namespace mir {
namespace tests {
namespace unit {


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


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
