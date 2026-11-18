// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "action.h"


namespace mir::action::tests {


// a global O8 field (not a land-sea mask) from a file, as mask with a threshold (masks don't need downloading)
const std::string LSM{
    "lsm: true, lsm-selection: file, lsm-file: 'gridType=reduced_gg,gridName=O8,shortName=q.grib2', "
    "lsm-value-threshold: 2.4e-6"};


size_t count_mask(const std::string& type, const std::string& grid, int value) {
    auto in = field(grid, [](double lat, double) { return lat; }, 2, 9999.);
    auto out =
        perform("{type: " + type + ", " + type.substr(7) + "-value: " + std::to_string(value) + ", " + LSM + "}", in);

    EXPECT_EQUAL(out.dimensions(), 2);
    EXPECT_EQUAL(count_missing(out, 0), count_missing(out, 1));

    return count_missing(out);
}


CASE("filter.mask-input-lsm, filter.mask-output-lsm") {
    const std::vector<std::string> types{"filter.mask-input-lsm", "filter.mask-output-lsm"};
    for (const std::string& type : types) {
        const std::vector<std::string> grids{"{grid: O8}", "{grid: [10, 10]}"};
        for (const std::string& grid : grids) {
            SECTION(type + " " + grid) {
                const auto N   = representation(grid)->numberOfPoints();
                const auto sea = count_mask(type, grid, 0);
                const auto lnd = count_mask(type, grid, 1);

                // masking either value covers all points once
                EXPECT(0 < sea && sea < N);
                EXPECT(0 < lnd && lnd < N);
                EXPECT_EQUAL(sea + lnd, N);
            }
        }
    }
}


CASE("filter.mask-input-lsm, with missing values") {
    auto in  = field("{grid: O8}", [](double lat, double) { return lat > 50 ? 9999. : lat; }, 1, 9999.);
    auto out = perform("{type: filter.mask-input-lsm, mask-input-lsm-value: 1, " + LSM + "}", in);

    EXPECT(out.hasMissing());
    EXPECT(count_missing(out) > count_missing(in));

    for (size_t i = 0; i < in.values(0).size(); ++i) {
        EXPECT(in.values(0)[i] != 9999. || out.values(0)[i] == 9999.);
    }
}


CASE("filter.mask-input-lsm, filter.mask-output-lsm, properties") {
    auto in0  = make("{type: filter.mask-input-lsm, mask-input-lsm-value: 0}");
    auto in1  = make("{type: filter.mask-input-lsm, mask_input_lsm_value: 1}");
    auto out0 = make("{type: filter.mask-output-lsm}");
    auto out1 = make("{type: filter.mask-output-lsm, mask-output-lsm-value: 1}");

    EXPECT_EQUAL(in1->json_str(), R"({"type":"filter.mask-input-lsm","mask-input-lsm-value":1})");
    EXPECT_EQUAL(out0->json_str(), R"({"type":"filter.mask-output-lsm","mask-output-lsm-value":0})");

    EXPECT(*in0 == *make_checked("filter.mask-input-lsm"));
    EXPECT(*in1 == *make_checked(in1->json_str(false)));
    EXPECT(*in0 != *in1);
    EXPECT(*out0 != *out1);
    EXPECT(*in0 != *out0);
    EXPECT(*in1 != *out1);
}


CASE("filter.mask-input-lsm, filter.mask-output-lsm, errors") {
    EXPECT_THROWS(make("{type: filter.mask-input-lsm, mask-input-lsm-value: 2}"));
    EXPECT_THROWS(make("{type: filter.mask-output-lsm, mask-output-lsm-value: -1}"));

    // lsm: true is required
    EXPECT_THROWS(perform("filter.mask-input-lsm", field("{grid: O32}")));
    EXPECT_THROWS(perform("filter.mask-output-lsm", field("{grid: O32}")));

    // lsm file is required
    EXPECT_THROWS(perform("{type: filter.mask-input-lsm, lsm: true, lsm-selection: file}", field("{grid: O32}")));
}


CASE("filter.mask-input-lsm, printing") {
    EXPECT(str(*make("{type: filter.mask-input-lsm, mask-input-lsm-value: 1}")).find("[value=1]") != std::string::npos);
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
