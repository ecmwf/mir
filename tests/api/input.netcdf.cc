// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/util/Log.h"

#include "input.h"


namespace mir::api::tests {


CASE("NetcdfFileInput") {
    auto in = build("nemo.nc", "checkDuplicatePoints: False");
    EXPECT(to_string(*in).find("NetcdfFileInput[") == 0);

    size_t count = 0;
    while (in->next()) {
        ++count;
        EXPECT(in->dimensions() > 0);

        const auto& param = in->parametrisation();
        Log::info() << "NetcdfFileInput: " << param << std::endl;

        // values from the field, or from the field rules
        bool b = false;
        EXPECT_NO_THROW(param.get("gridded", b));

        std::vector<double> grid;
        EXPECT_NO_THROW(param.get("grid", grid));

        bool checkDuplicatePoints = true;
        EXPECT(param.get("checkDuplicatePoints", checkDuplicatePoints) && !checkDuplicatePoints);

        auto field = in->field();
        EXPECT_EQUAL(field.dimensions(), in->dimensions());
        EXPECT_EQUAL(field.values(0).size(), number_of_values(field));
    }

    EXPECT(count > 0);
    EXPECT(in->sameAs(*build("nemo.nc")));
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
