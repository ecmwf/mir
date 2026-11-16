// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <sstream>
#include <vector>

#include "eckit/log/Log.h"

#include "mir/repres/UnstructuredGrid.h"

#include "action.h"


namespace mir::action::tests {


std::string add_random(const std::string& distribution) {
    return "{type: filter.add-random, add-random: '" + distribution + "'}";
}


CASE("filter.add-random") {
    auto in = field("{grid: [10, 10]}", nullptr, 2);

    SECTION("uniform-int-distribution") {
        auto out = perform(*make(add_random("uniform-int-distribution{a:1,b:3}")), in);

        EXPECT_EQUAL(out.dimensions(), 2);

        for (size_t which = 0; which < 2; ++which) {
            const auto& v = out.values(which);

            EXPECT_EQUAL(v.size(), in.values(which).size());

            size_t count[3]{0, 0, 0};

            for (const auto& x : v) {
                EXPECT(x == 1. || x == 2. || x == 3.);

                count[static_cast<size_t>(x) - 1]++;
            }

            // 684 samples: all values are drawn
            EXPECT(count[0] > 0 && count[1] > 0 && count[2] > 0);
        }
    }

    SECTION("normal-distribution") {
        auto out = perform(*make(add_random("normal-distribution{mean:42.,stddev:3.}")), in);

        // 684 samples: the mean standard error is about 0.11
        const auto& v   = out.values(0);
        const auto mean = std::accumulate(v.begin(), v.end(), 0.) / static_cast<double>(v.size());

        EXPECT(std::abs(mean - 42.) < 1.);
    }

    SECTION("bernoulli-distribution, default arguments") {
        auto out = perform(*make(add_random("bernoulli-distribution{}")), in);

        for (const auto& x : out.values(0)) {
            EXPECT(x == 0. || x == 1.);
        }
    }
}


CASE("filter.add-random, with missing values") {
    auto in  = field("{grid: [10, 10]}", [](double lat, double) { return lat > 50 ? 9999. : 0.; }, 1, 9999.);
    auto out = perform(*make(add_random("uniform-int-distribution{a:1,b:3}")), in);

    EXPECT(out.hasMissing());
    EXPECT_EQUAL(count_missing(out), count_missing(in));

    for (size_t i = 0; i < in.values(0).size(); ++i) {
        const auto x = out.values(0)[i];

        EXPECT(in.values(0)[i] == 9999. ? x == 9999. : (1. <= x && x <= 3.));
    }
}


CASE("filter.add-random, properties") {
    auto a = make(add_random("uniform-int-distribution{a:1,b:3}"));

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.add-random","add-random":"uniform-int-distribution{a:1,b:3}"})");

    auto b = make(a->json_str(false));

    EXPECT_EQUAL(b->json_str(false), a->json_str(false));
}


CASE("filter.add-random, errors") {
    EXPECT_THROWS(make("filter.add-random"));  // add-random is required
    EXPECT_THROWS(make(add_random("unknown-distribution{}")));
}


// ICE.1 is 17 rows (north to south) by 43 columns, rows 8 to 12 off for columns 12 to 32
const std::string ICE{"{grid: [1, 1], area: [16, 0, 0, 42]}"};


CASE("filter.bitmap") {
    auto in = field(ICE, [](double lat, double lon) { return lat + lon; }, 2, 9999.);

    EXPECT_EQUAL(in.values(0).size(), 17 * 43);
    EXPECT(!in.hasMissing());

    auto out = perform("{type: filter.bitmap, bitmap: ICE.1}", in);

    EXPECT_EQUAL(out.dimensions(), 2);
    EXPECT(out.hasMissing());

    for (size_t which = 0; which < 2; ++which) {
        EXPECT_EQUAL(count_missing(out, which), 5 * 21);

        for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
            const auto& p  = it->pointUnrotated();
            const auto row = static_cast<size_t>(16. - p.lat().value()) + 1;
            const auto col = static_cast<size_t>(p.lon().value()) + 1;
            const auto v   = out.values(which)[it->index()];

            const bool off = 8 <= row && row <= 12 && 12 <= col && col <= 32;

            EXPECT(off ? v == 9999. : same(v, p.lat().value() + p.lon().value()));
        }
    }
}


CASE("filter.bitmap, smaller field") {
    // fewer values than the bitmap: masking follows the bitmap rows, up to the number of values
    auto in  = field("{grid: [1, 1], area: [16, 0, 8, 42]}", [](double lat, double) { return lat; });
    auto out = perform("{type: filter.bitmap, bitmap: ICE.1}", in);

    EXPECT_EQUAL(in.values(0).size(), 9 * 43);
    EXPECT_EQUAL(count_missing(out), 2 * 21);
}


CASE("filter.bitmap, other bitmaps") {
    struct test_t {
        std::string bitmap;
        std::string grid;
    };

    const std::vector<test_t> tests{
        test_t{"GL05", "{grid: [1, 1], area: [52, 0, 0, 140]}"},
        test_t{"GRE01.MAP", "{grid: [1, 1], area: [80, 0, 0, 110]}"},
    };
    for (const auto& test : tests) {
        SECTION(test.bitmap) {
            auto in  = field(test.grid, [](double lat, double) { return lat; });
            auto out = perform("{type: filter.bitmap, bitmap: " + test.bitmap + "}", in);

            EXPECT(out.hasMissing());
            EXPECT(count_missing(out) > 0);
            EXPECT(count_missing(out) < in.values(0).size());
        }
    }
}


CASE("filter.bitmap, properties") {
    auto a = make("{type: filter.bitmap, bitmap: ICE.1}");

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.bitmap","bitmap":"ICE.1"})");
    EXPECT(*a == *make("{type: filter.bitmap, bitmap: ICE.1}"));
    EXPECT(*a != *make("{type: filter.bitmap, bitmap: GL05}"));
}


CASE("filter.bitmap, errors") {
    EXPECT_THROWS(make("filter.bitmap"));  // bitmap is required

    // more values than the bitmap
    EXPECT_THROWS(perform("{type: filter.bitmap, bitmap: ICE.1}", field("{grid: [1, 1]}")));

    EXPECT_THROWS(perform("{type: filter.bitmap, bitmap: does-not-exist}", field(ICE)));
}


CASE("filter.frame") {
    struct test_t {
        std::string grid;
        size_t frame;
        size_t missing;
    };

    // O16 rows have 20, 24, ..., 80 (then symmetric) points, the interior excludes 2 rows/points on each side
    size_t O16 = 0;

    for (size_t j = 2; j < 16; ++j) {
        O16 += 2 * (20 + 4 * j - 4);
    }

    const std::vector<test_t> tests{
        test_t{"{grid: [10, 10]}", 2, (36 - 4) * (19 - 4)},
        test_t{"{grid: [10, 10]}", 1, (36 - 2) * (19 - 2)},
        test_t{"{grid: [10, 10], area: [60, 0, -30, 350]}", 3, (36 - 6) * (10 - 6)},
        test_t{"{grid: F16}", 2, (64 - 4) * (32 - 4)},
        test_t{"{grid: O16}", 2, O16},
    };
    for (const auto& test : tests) {
        SECTION(test.grid + " frame: " + std::to_string(test.frame)) {
            auto in  = field(test.grid, [](double lat, double) { return lat; }, 2);
            auto out = perform("{type: filter.frame, frame: " + std::to_string(test.frame) + "}", in);

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT(out.hasMissing());
            EXPECT_EQUAL(count_missing(out, 0), test.missing);
            EXPECT_EQUAL(count_missing(out, 1), test.missing);

            // the frame keeps its values
            for (size_t i = 0; i < in.values(0).size(); ++i) {
                const auto v = out.values(0)[i];

                EXPECT(v == 9999. || v == in.values(0)[i]);
            }
        }
    }
}


CASE("filter.frame, no effect") {
    // a frame wider than the grid keeps all values
    auto in  = field("{grid: [10, 10]}", [](double lat, double) { return lat; });
    auto out = perform("{type: filter.frame, frame: 10}", in);

    EXPECT(!out.hasMissing());
    EXPECT(out.values(0) == in.values(0));
}


CASE("filter.frame, properties") {
    auto a = make("{type: filter.frame, frame: 2}");

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.frame","frame":2})");
    EXPECT(*a == *make("{type: filter.frame, frame: 2}"));
    EXPECT(*a != *make("{type: filter.frame, frame: 3}"));
}


CASE("filter.frame, errors") {
    EXPECT_THROWS(make("filter.frame"));  // frame is required

    // not supported
    EXPECT_THROWS(perform("{type: filter.frame, frame: 2}", field("{grid: H8}")));
}


CASE("filter.globalise") {
    SECTION("regional, periodic west-east") {
        auto in = field("{grid: [10, 10], area: [60, 0, -30, 350]}", [](double lat, double) { return lat; }, 2);

        EXPECT_EQUAL(in.values(0).size(), 10 * 36);
        EXPECT(!in.representation()->isGlobal());

        auto out = perform("filter.globalise", in);

        EXPECT(out.representation()->isGlobal());
        EXPECT_EQUAL(out.dimensions(), 2);
        EXPECT_EQUAL(out.values(0).size(), 19 * 36);
        EXPECT(out.hasMissing());

        // new rows have missing values, old rows keep their values
        for (size_t which = 0; which < 2; ++which) {
            EXPECT_EQUAL(count_missing(out, which), 9 * 36);

            for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
                const auto lat = it->pointUnrotated().lat().value();
                const auto v   = out.values(which)[it->index()];

                EXPECT(lat > 60 || lat < -30 ? v == 9999. : same(v, lat));
            }
        }
    }

    const std::vector<std::string> grids{"{grid: [10, 10]}", "{grid: O16}"};
    for (const std::string& grid : grids) {
        SECTION("global " + grid + ": no effect") {
            auto in  = field(grid, [](double lat, double) { return lat; });
            auto out = perform("filter.globalise", in);

            EXPECT(out.representation()->sameAs(*in.representation()));
            EXPECT(out.values(0) == in.values(0));
            EXPECT(!out.hasMissing());
        }
    }
}


CASE("filter.globalise, properties") {
    std::unique_ptr<action::Action> a(action::ActionFactory::make_from_string("filter.globalise"));

    EXPECT(a->deleteWithNext(*make("{type: crop.area, area: [10, -10, -10, 10]}")));
    EXPECT(!a->deleteWithNext(*make("{type: mask.area, area: [10, -10, -10, 10]}")));
    EXPECT(!a->deleteWithNext(*make("filter.globalise")));

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.globalise"})");
    EXPECT(*a == *make_checked("{type: filter.globalise}"));
    EXPECT(*a != *make("filter.unstructured"));
}


CASE("filter.globalise, errors") {
    // regional, not periodic west-east
    EXPECT_THROWS(perform("filter.globalise", field("{grid: [10, 10], area: [60, 0, -30, 100]}")));
}


CASE("filter.limiter") {
    auto in = field("{grid: [10, 10]}", [](double lat, double) { return lat; });

    SECTION("minimum and maximum") {
        auto out      = perform("{type: filter.limiter, limiter-minimum: -10, limiter-maximum: 20}", in);
        const auto& v = out.values(0);

        EXPECT(*std::min_element(v.begin(), v.end()) == -10.);
        EXPECT(*std::max_element(v.begin(), v.end()) == 20.);
    }

    SECTION("missing values are kept") {
        auto miss = field("{grid: [10, 10]}", [](double lat, double) { return lat > 50 ? 9999. : lat; }, 1, 9999.);
        auto out  = perform("{type: filter.limiter, limiter-minimum: 0, limiter-maximum: 1}", miss);

        EXPECT_EQUAL(count_missing(out), count_missing(miss));
    }

    SECTION("as configured, by parameter") {
        auto out      = perform("{type: filter.limiter, paramId: 29}", in);
        const auto& v = out.values(0);

        EXPECT(*std::min_element(v.begin(), v.end()) == 1.);
        EXPECT(*std::max_element(v.begin(), v.end()) == 20.);
    }

    SECTION("as configured, by parameter, as given") {
        auto out      = perform("{type: filter.limiter, paramId: 29, limiter-minimum: 0, limiter-maximum: 2}", in);
        const auto& v = out.values(0);

        EXPECT(*std::min_element(v.begin(), v.end()) == 0.);
        EXPECT(*std::max_element(v.begin(), v.end()) == 2.);
    }

    SECTION("as configured, unknown parameter: unlimited") {
        auto out = perform("{type: filter.limiter, paramId: 167}", in);

        EXPECT(out.values(0) == in.values(0));
    }
}


CASE("filter.limiter, errors") {
    EXPECT_THROWS(make("{type: filter.limiter, limiter-minimum: 1, limiter-maximum: 0}"));
}


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


/// Statistics (as logged) of the action performed on the field, which is kept unchanged
std::string statistics(const std::string& spec, const data::MIRField& in) {
    std::ostringstream log;

    struct capture_t {
        explicit capture_t(std::ostream& log) { eckit::Log::info().setStream(log); }
        ~capture_t() { eckit::Log::info().setStream(std::cout); }
    };

    auto out = [&]() {
        capture_t capture(log);
        return perform(spec, in);
    }();

    EXPECT_EQUAL(out.dimensions(), in.dimensions());
    EXPECT(out.values(0) == in.values(0));
    EXPECT_EQUAL(out.hasMissing(), in.hasMissing());

    return log.str();
}


bool contains(const std::string& str, const std::string& sub) {
    if (str.find(sub) != std::string::npos) {
        return true;
    }

    std::cerr << "'" << sub << "' not found in '" << str << "'" << std::endl;
    return false;
}


CASE("filter.statistics") {
    auto in = field("{grid: [10, 10]}", [](double lat, double) { return lat; });

    SECTION("min-max") {
        auto log = statistics("{type: filter.statistics, statistics: min-max}", in);

        EXPECT(contains(log, "count=684,missing=0"));
        EXPECT(contains(log, "max=90"));
        EXPECT(contains(log, "min=-90"));
    }

    SECTION("scalar") {
        auto log = statistics("{type: filter.statistics, statistics: scalar}", in);

        EXPECT(contains(log, "count=684,missing=0"));
        EXPECT(contains(log, ",stddev=54.8123,"));
    }

    SECTION("several") {
        auto log = statistics("{type: filter.statistics, statistics: min-max/p-norms/central-moments}", in);

        EXPECT(contains(log, "min=-90"));
        EXPECT(contains(log, "L1"));
    }

    SECTION("input/output statistics") {
        auto log = statistics("{type: filter.statistics, which-statistics: output, output-statistics: min-max}", in);

        EXPECT(contains(log, "min=-90"));
    }

    SECTION("precision") {
        auto in2 = field("{grid: [10, 10]}", [](double, double) { return 1. / 3.; });
        auto log = statistics("{type: filter.statistics, statistics: min-max, precision: 3}", in2);

        EXPECT(contains(log, "max=0.333,"));
    }
}


CASE("filter.statistics, with missing values") {
    auto in  = field("{grid: [10, 10]}", [](double lat, double) { return lat > 50 ? 9999. : lat; }, 1, 9999.);
    auto log = statistics("{type: filter.statistics, statistics: min-max}", in);

    EXPECT(contains(log, "count=684,missing=" + std::to_string(count_missing(in))));
    EXPECT(contains(log, "max=50"));
}


CASE("filter.statistics, spectral") {
    MIRValuesVector values((10 + 1) * (10 + 2), 1.);
    auto log = statistics("{type: filter.statistics, statistics: spectral}", field("{grid: T10}", values));

    EXPECT(!log.empty());
}


CASE("filter.statistics, properties") {
    auto a = make_checked("{type: filter.statistics, statistics: min-max, precision: 3}");

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.statistics","statistics":"min-max","precision":3})");

    EXPECT(*a == *make("{type: filter.statistics, which-statistics: input, input-statistics: min-max}"));
    EXPECT(*a != *make("{type: filter.statistics, statistics: scalar}"));
    EXPECT(*a != *make("{type: filter.statistics, statistics: min-max/scalar}"));
    EXPECT(*make("{type: filter.statistics, statistics: min-max/scalar}") !=
           *make("{type: filter.statistics, statistics: min-max/p-norms}"));
}


CASE("filter.statistics, errors") {
    EXPECT_THROWS(make("filter.statistics"));  // statistics is required
    EXPECT_THROWS(make("{type: filter.statistics, statistics: unknown}"));

    // one dimension only
    EXPECT_THROWS(perform("{type: filter.statistics, statistics: min-max}", field("{grid: [10, 10]}", nullptr, 2)));
}


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


CASE("filter.*, printing") {
    auto starts = [](const std::string& str, const std::string& prefix) { return str.rfind(prefix, 0) == 0; };

    EXPECT_EQUAL(str(*make("{type: filter.limiter, limiter-minimum: -10, limiter-maximum: 20}")),
                 "Limiter[minimum=-10,maximum=20]");
    EXPECT_EQUAL(str(*make("{type: filter.frame, frame: 2}")), "FrameFilter[size=2]");
    EXPECT_EQUAL(str(*make("filter.globalise")), "GlobaliseFilter[]");
    EXPECT_EQUAL(str(*make("filter.unstructured")), "UnstructuredFilter[]");

    EXPECT(starts(str(*make(add_random("uniform-int-distribution{a:1,b:3}"))), "AddRandomFilter[distribution="));

    auto bitmap = make("{type: filter.bitmap, bitmap: ICE.1}");

    EXPECT(starts(str(*bitmap), "BitmapFilter[path=") && str(*bitmap).find("ICE.1") != std::string::npos);
    EXPECT_EQUAL(custom_str(*bitmap), "BitmapFilter[path=...]");

    EXPECT(str(*make("{type: filter.mask-input-lsm, mask-input-lsm-value: 1}")).find("[value=1]") != std::string::npos);

    const auto statistics = str(*make("{type: filter.statistics, statistics: min-max/scalar}"));

    EXPECT(starts(statistics, "StatisticsFilter[") && statistics.back() == ']');

    // printing is the default plan dump
    EXPECT_EQUAL(custom_str(*make("{type: filter.frame, frame: 2}")), "FrameFilter[size=2]");
}


CASE("filter.add-random, comparison") {
    // random values are never the same, so neither are the actions
    auto a = make(add_random("uniform-int-distribution{a:1,b:3}"));

    EXPECT(*a != *a);
    EXPECT(*a != *make(add_random("uniform-int-distribution{a:1,b:3}")));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    // limits by parameter, as configured for testing (filter.limiter)
    ::setenv("MIR_CONFIG_LIMITER", "~mir/etc/mir/limiter-test.yaml", 1);
    return eckit::testing::run_tests(argc, argv);
}
