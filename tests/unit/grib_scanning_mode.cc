// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <map>
#include <string>

#include "mir/util/Grib.h"

#include "eckit/testing/Test.h"


namespace mir::tests::unit {


CASE("GRIB scanningMode <-> order") {
    static const std::map<std::string, long> modes{
        {"i+j+", 64},    //
        {"i+j-", 0},     //
        {"i-j+", 192},   //
        {"i-j-", 128},   //
        {"i-+j+", 208},  //
        {"i-+j-", 144},  //
        {"i+-j+", 80},   //
        {"i+-j-", 16},   //
        {"j+i+", 96},    //
        {"j+i-", 224},   //
        {"j-i+", 32},    //
        {"j-i-", 160},   //
        {"j-+i+", 48},   //
        {"j-+i-", 176},  //
        {"j+-i+", 112},  //
        {"j+-i-", 240},  //
    };

    for (const auto& [order, scanning_mode] : modes) {
        EXPECT(grib_order_to_scanning_mode(order) == scanning_mode);
        EXPECT(grib_order(order) == order);
    }

    // the default (scanningMode=0), also of the empty order
    EXPECT(grib_order_default() == "i+j-");
    EXPECT(grib_order("") == grib_order_default());

    // unknown orders
    for (const std::string order : {"", "i+j", "j-", "i+i-", "i+j-+"}) {
        EXPECT_THROWS(grib_order_to_scanning_mode(order));
    }
    EXPECT_THROWS(grib_order("i+-"));
}


}  // namespace mir::tests::unit

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
