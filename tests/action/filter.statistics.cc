// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <iostream>
#include <sstream>

#include "eckit/log/Log.h"

#include "action.h"


namespace mir::action::tests {


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


CASE("filter.statistics, printing") {
    const auto statistics = str(*make("{type: filter.statistics, statistics: min-max/scalar}"));

    EXPECT(statistics.rfind("StatisticsFilter[", 0) == 0 && statistics.back() == ']');
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
