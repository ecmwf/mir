// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "mir/data/MIRField.h"
#include "mir/input/GribMemoryInput.h"
#include "mir/input/MIRInput.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"

#include "eckit/testing/Test.h"


namespace mir::tests::unit {


CASE("space_view: as the eccodes iterator") {
    // sectors of GRIB2 space_view (Nr=6.6107, dx=3622), points as the eccodes iterator (first, second and last)
    using keys_t   = std::vector<std::pair<std::string, long>>;
    using points_t = std::vector<std::pair<double, double>>;  // (lat, lon)

    const keys_t sector{{"Nx", 16},      {"Ny", 16},   {"dx", 3622}, {"dy", 3622},   {"Xp", 1856000},
                        {"Yp", 1856000}, {"Xo", 1500}, {"Yo", 1700}, {"Nr", 6610700}};

    for (const auto& [keys, points] : std::vector<std::pair<keys_t, points_t>>{
             {{{"shapeOfTheEarth", 6}},
              {{4.223693723, 350.299524805}, {4.223622147, 350.327193534}, {3.815601110, 350.719698129}}},
             {{{"shapeOfTheEarth", 6}, {"iScansNegatively", 1}, {"jScansPositively", 1}},
              {{-3.815601110, 9.280301871}, {-3.815663064, 9.307903835}, {-4.223693723, 9.700475195}}},
             {{{"shapeOfTheEarth", 5},
               {"longitudeOfSubSatellitePoint", -75000000},
               {"Ny", 12},
               {"dy", 3610},
               {"Xo", 2400},
               {"Yo", 1200}},
              {{18.425011761, 300.905565350}, {18.425558651, 300.936080607}, {18.106822780, 301.327135202}}},
         }) {
        std::unique_ptr<codes_handle, decltype(&codes_handle_delete)> h(
            codes_grib_handle_new_from_samples(nullptr, "GRIB2"), &codes_handle_delete);
        ASSERT(h);

        // as grib_filter: values (one) before the number of points
        GRIB_CALL(codes_set_string(h.get(), "gridType", "space_view", nullptr));

        auto all(sector);
        all.insert(all.end(), keys.begin(), keys.end());
        for (const auto& [key, value] : all) {
            GRIB_CALL(codes_set_long(h.get(), key.c_str(), value));
        }

        long nx = 0;
        long ny = 0;
        GRIB_CALL(codes_get_long(h.get(), "Nx", &nx));
        GRIB_CALL(codes_get_long(h.get(), "Ny", &ny));

        const double one = 1.;
        GRIB_CALL(codes_set_double_array(h.get(), "values", &one, 1));
        GRIB_CALL(codes_set_long(h.get(), "numberOfDataPoints", nx * ny));
        GRIB_CALL(codes_set_long(h.get(), "numberOfValues", nx * ny));

        const void* message = nullptr;
        size_t length       = 0;
        GRIB_CALL(codes_get_message(h.get(), &message, &length));

        std::unique_ptr<input::MIRInput> input(new input::GribMemoryInput(message, length));
        const auto field = input->field();
        std::unique_ptr<repres::Iterator> it(field.representation()->iterator());

        points_t visited;
        while (it->next()) {
            const auto& p = it->pointUnrotated();
            visited.emplace_back(p.lat().value(), p.lon().value());
        }

        EXPECT_EQUAL(visited.size(), static_cast<size_t>(nx * ny));

        for (const auto& [p, q] : std::vector<std::pair<std::pair<double, double>, std::pair<double, double>>>{
                 {visited.front(), points[0]}, {visited[1], points[1]}, {visited.back(), points[2]}}) {
            // longitudes are fractions (mir::Longitude), which approximate (to about 1e-5 degrees)
            EXPECT(std::abs(p.first - q.first) < 1e-6);
            EXPECT(std::abs(std::remainder(p.second - q.second, 360.)) < 1e-5);
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
