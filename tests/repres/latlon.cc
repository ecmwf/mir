// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"

#include "repres.h"


namespace mir::repres::tests {


using latlon::LatLon;
using latlon::RegularLL;
using util::BoundingBox;
using util::Increments;


CASE("LatLon::globaliseBoundingBox") {
    struct test_t {
        std::string name;
        Increments increments;
        BoundingBox area;
        BoundingBox global;
        size_t Ni;
        size_t Nj;
    };

    const std::vector<test_t> tests{
        {"001", Increments{1, 1}, {0, -350, 0, 8}, {90, 0, -90, 359}, 360, 181},
        {"002", Increments{0.5, 0.5}, {0, -350, 0, 9}, {90, 0, -90, 359.5}, 720, 361},
        {"003", Increments{2, 2}, {2, 0, 0, 2}, {90, 0, -90, 358}, 180, 91},
        {"004", Increments{2, 2}, {2.1, 0, 0, 2.1}, {90, 0, -90, 358}, 180, 91},
        {"005", Increments{2, 2}, {3, 1, 1, 3}, {89, 1, -89, 359}, 180, 90},
        {"006",
         Increments{2, 2},
         {37.6025, -114.8907, 27.7626, -105.1875},
         {89.7626, 1.1093, -88.2374, 359.1093},
         180,
         90},
        {"007", Increments{2, 2}, {88, -178, -88, 180}, {90, 0, -90, 358}, 180, 91},
        {"008", Increments{7, 7}, {85, 0, -90, 357}, {85, 0, -90, 357}, 52, 26},  // not periodic
        {"009", Increments{7, 7}, BoundingBox{}, {85, 0, -90, 357}, 52, 26},
        {"010", Increments{4, 4}, BoundingBox{}, {90, 0, -90, 356}, 90, 46},
    };

    for (const auto& test : tests) {
        SECTION(test.name) {
            PointLatLon reference(test.area.south(), test.area.west());

            BoundingBox global(test.area);
            LatLon::globaliseBoundingBox(global, test.increments, reference);

            EXPECT(same(global.north().value(), test.global.north().value(), 1e-6));
            EXPECT(same(global.west().value(), test.global.west().value(), 1e-6));
            EXPECT(same(global.south().value(), test.global.south().value(), 1e-6));
            EXPECT(same(global.east().value(), test.global.east().value(), 1e-6));

            RepresentationHandle repres(new RegularLL(test.increments, global, reference));
            const auto& ll = dynamic_cast<const RegularLL&>(*repres);

            EXPECT_EQUAL(ll.Ni(), test.Ni);
            EXPECT_EQUAL(ll.Nj(), test.Nj);
        }
    }
}


CASE("LatLon::globalise (towards the poles)") {
    RepresentationHandle repres(new RegularLL(Increments{10, 10}, {40, 0, -40, 350}));
    EXPECT_EQUAL(repres->numberOfPoints(), 36 * 9);

    MIRValuesVector values(36 * 9);
    for (size_t i = 0; i < values.size(); ++i) {
        values[i] = static_cast<double>(i);
    }

    data::MIRField field(repres, false, 9999.);
    field.update(values, 0);

    RepresentationHandle global(repres->globalise(field));
    ASSERT(global);

    EXPECT(global->isGlobal());
    EXPECT_EQUAL(global->numberOfPoints(), 36 * 19);

    // rows north of 40 are missing values (5 rows of 36), followed by the values
    const auto& v = field.values(0);
    EXPECT_EQUAL(v.size(), 36 * 19);
    EXPECT(field.hasMissing());
    EXPECT_EQUAL(v[0], 9999.);
    EXPECT_EQUAL(v[5 * 36 - 1], 9999.);
    EXPECT_EQUAL(v[5 * 36], 0.);
    EXPECT_EQUAL(v[5 * 36 + 36 * 9 - 1], 36. * 9. - 1.);
    EXPECT_EQUAL(v[5 * 36 + 36 * 9], 9999.);

    // already global
    data::MIRField again(global, true, 9999.);
    again.update(field.direct(0), 0);
    EXPECT(global->globalise(again) == nullptr);
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
