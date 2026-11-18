// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/JSON.h"
#include "eckit/testing/Test.h"

#include "mir/param/DefaultParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"

#include "parametrisation.h"


namespace mir::api::tests {


CASE("DefaultParametrisation") {
    const auto& defaults = param::DefaultParametrisation::instance();

    std::string interpolation;
    EXPECT(defaults.get("interpolation", interpolation) && interpolation == "linear");

    long nclosest = 0;
    EXPECT(defaults.get("nclosest", nclosest) && nclosest == 4);


    SECTION("get_value") {
        SimpleParametrisation user;
        EXPECT_EQUAL(defaults.get_value<long>("nclosest", user), 4);
        EXPECT_EQUAL(defaults.get_value<size_t>("nclosest", user), 4);
        EXPECT_EQUAL(defaults.get_value<double>("distance", user), 1.);

        user.set("nclosest", 8L);
        EXPECT_EQUAL(defaults.get_value<long>("nclosest", user), 8);
        EXPECT_EQUAL(defaults.get_value<size_t>("nclosest", user), 8);

        // unknown default
        EXPECT_THROWS_AS(defaults.get_value<long>("unknown", user), eckit::AssertionFailed);
    }


    SECTION("json (values different from the defaults, or all if complete)") {
        auto json = [&defaults]() {
            std::ostringstream str;
            eckit::JSON j(str);
            j.startObject();
            defaults.json(j, "nclosest", 4L);
            defaults.json(j, "nclosest", size_t(8));
            defaults.json(j, "distance", 1.);
            defaults.json(j, "distance", 2.);
            defaults.json(j, "interpolation", std::string("linear"));
            defaults.json(j, "unknown", 1L);
            j.endObject();
            return str.str();
        };

        EXPECT_NOT(param::DefaultParametrisation::json_complete());
        EXPECT_EQUAL(json(), R"({"nclosest":8,"distance":2,"unknown":1})");

        {
            param::DefaultParametrisation::JSONComplete complete;
            EXPECT(param::DefaultParametrisation::json_complete());
            EXPECT_EQUAL(
                json(),
                R"({"nclosest":4,"nclosest":8,"distance":1,"distance":2,"interpolation":"linear","unknown":1})");
        }

        EXPECT_NOT(param::DefaultParametrisation::json_complete());
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
