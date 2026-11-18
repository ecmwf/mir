// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "mir/repres/UnstructuredGrid.h"

#include "action.h"


namespace mir::action::tests {


CASE("filter.unstructured") {
    const std::vector<std::string> grids{
        "{grid: [10, 10]}", "{grid: [10, 10], area: [60, 0, -30, 100]}",
        "{grid: O16}",      "{grid: F16}",
        "{grid: H8}",       "{grid: [1, 1], area: [10, -10, -10, 10], rotation: [-40, 22]}"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            auto in  = field(grid, [](double lat, double lon) { return lat + lon; }, 2, 9999.);
            auto out = perform("filter.unstructured", in);

            EXPECT(dynamic_cast<const repres::UnstructuredGrid*>(in.representation()) == nullptr);
            EXPECT(dynamic_cast<const repres::UnstructuredGrid*>(out.representation()) != nullptr);

            // same points, in the same order, and the same values
            const auto N = in.representation()->numberOfPoints();

            EXPECT_EQUAL(out.representation()->numberOfPoints(), N);
            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT(out.values(0) == in.values(0));
            EXPECT(out.values(1) == in.values(1));

            // geographic coordinates (if rotated, not the grid coordinates)
            std::vector<Point2> points(N);

            for (std::unique_ptr<repres::Iterator> it(in.representation()->iterator()); it->next();) {
                points.at(it->index()) = **it;
            }

            for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
                const auto& p = it->pointUnrotated();
                const auto& q = points.at(it->index());

                EXPECT(same(p.lat().value(), q[0], 1e-6));
                EXPECT(same(std::remainder(p.lon().value() - q[1], 360.), 0., 1e-6));
            }
        }
    }
}


CASE("filter.unstructured, with missing values") {
    auto in  = field("{grid: [10, 10]}", [](double lat, double) { return lat > 50 ? 9999. : lat; }, 1, 9999.);
    auto out = perform("filter.unstructured", in);

    EXPECT(out.hasMissing());
    EXPECT(out.values(0) == in.values(0));
}


CASE("filter.unstructured, properties") {
    auto a = make("filter.unstructured");

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.unstructured"})");
    EXPECT(*a == *make_checked("{type: filter.unstructured}"));
    EXPECT(*a != *make("filter.globalise"));
}


CASE("filter.unstructured, printing") {
    EXPECT_EQUAL(str(*make("filter.unstructured")), "UnstructuredFilter[]");
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
