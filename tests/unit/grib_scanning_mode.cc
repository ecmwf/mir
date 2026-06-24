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


#include <map>
#include <string>

#include "eckit/geo/order/Scan.h"

#include "mir/util/Grib.h"

#include "eckit/testing/Test.h"


namespace mir::tests::unit {


CASE("GRIB scanningMode <-> eckit::geo::order::Scan") {
    using eckit::geo::order::Scan;

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
        {"j-+i+", 32},   //
        {"j-+i-", 160},  //
        {"j+-i+", 96},   //
        {"j+-i-", 224},  //
    };

    for (const auto& [order, scanning_mode] : modes) {
        EXPECT(grib_order_to_scanning_mode(order) == scanning_mode);

        const Scan scan(order);
        EXPECT(scan.is_scan_i_then_j() == (order.front() == 'i'));
        EXPECT(scan.is_scan_i_positive() == (order.find("i+") != std::string::npos));
        EXPECT(scan.is_scan_j_positive() == (order.find("j+") != std::string::npos));

        bool alternating = order.find("+-") != std::string::npos || order.find("-+") != std::string::npos;
        EXPECT(scan.is_scan_alternating() == alternating);
    }
}


}  // namespace mir::tests::unit

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
