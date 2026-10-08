// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <string>

#include "eckit/testing/Test.h"

#include "mir/input/GribFileInput.h"


namespace mir::tests::unit {


// field (first message) setting, from grid-type.yaml or the grid catalog
std::string field_setting(const std::string& path, const std::string& name) {
    std::unique_ptr<input::MIRInput> input(new input::GribFileInput(path));
    ASSERT(input->next());

    std::string value;
    return input->parametrisation().get(name, value) ? value : "";
}


CASE("grid type settings") {
    SECTION("type=orca (gridType=unstructured_grid, uid from the grid catalog)") {
        const std::string path = "../data/ORCA.grib2";  // eORCA1_T

        EXPECT_EQUAL(field_setting(path, "intgrid"), "O96");  // from the grid catalog
        EXPECT_EQUAL(field_setting(path, "intermediate-interpolation"), "nn");
    }

    SECTION("other types") {
        const std::string path = "../data/gridType=reduced_gg,gridName=O32,shortName=msl.grib1";

        EXPECT_EQUAL(field_setting(path, "intgrid"), "");
        EXPECT_EQUAL(field_setting(path, "intermediate-interpolation"), "");
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
