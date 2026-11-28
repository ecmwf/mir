// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/TmpDir.h"
#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/input/GriddefInput.h"
#include "mir/output/GriddefOutput.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"

#include "input.h"


namespace mir::api::tests {


CASE("GriddefInput") {
    SECTION("text") {
        input_t in(new input::GriddefInput("tour.pts"));
        EXPECT(in->next());
        EXPECT_NOT(in->next());
        EXPECT_EQUAL(in->dimensions(), 1);

        bool gridded = false;
        EXPECT(in->parametrisation().get("gridded", gridded) && gridded);

        repres::RepresentationHandle repres(in->field().representation());
        EXPECT_EQUAL(repres->numberOfPoints(), 34);

        EXPECT_NOT(in->sameAs(*in));
        EXPECT_EQUAL(to_string(*in), "GriddefInput[size=34]");
    }


    SECTION("binary") {
        eckit::TmpDir dir;
        auto path = dir / "griddef";

        const std::vector<double> lats{10., 20., 30.};
        const std::vector<double> lons{1., 2., 3.};
        output::GriddefOutput::save(path, lats, lons, true);

        std::vector<double> latitudes;
        std::vector<double> longitudes;
        input::GriddefInput::load(path, latitudes, longitudes);
        EXPECT(latitudes == lats);
        EXPECT(longitudes == lons);

        input_t in(new input::GriddefInput(path));
        EXPECT_EQUAL(to_string(*in), "GriddefInput[size=3]");
    }


    SECTION("errors") {
        std::vector<double> latitudes;
        std::vector<double> longitudes;
        EXPECT_THROWS_AS(input::GriddefInput::load("does-not-exist.pts", latitudes, longitudes),
                         exception::CantOpenFile);
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
