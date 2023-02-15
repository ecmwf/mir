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


#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/data/Space.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tests::unit {


CASE("vector-space") {

    static const std::string key = "vector-space";
    static const param::DefaultParametrisation defaults;

    std::string defaultSpace;
    ASSERT(defaults.get(key, defaultSpace));

    auto combined = [](const param::MIRParametrisation& user, const param::MIRParametrisation& input,
                       const param::MIRParametrisation& defaults) {
        return std::unique_ptr<const param::MIRParametrisation>(
            new param::CombinedParametrisation(user, input, defaults));
    };

    std::vector<std::pair<std::string, size_t>> tests{{"1d-angle-degree-asymmetric", 1},
                                                      {"1d-angle-degree-symmetric", 1},
                                                      {"1d-angle-radian-asymmetric", 1},
                                                      {"1d-angle-radian-symmetric", 1},
                                                      {"1d-linear", 1},
                                                      {"1d-logarithmic", 1},
                                                      {"2d-vector-u", 2},
                                                      {"2d-vector-v", 2},
                                                      {"3d-cartesian-x", 3},
                                                      {"3d-cartesian-y", 3},
                                                      {"3d-cartesian-z", 3},
                                                      {"3d-vector-u", 3},
                                                      {"3d-vector-v", 3},
                                                      {"3d-vector-w", 3}};


    SECTION("vector-space default") {
        const param::SimpleParametrisation user;   // empty
        const param::SimpleParametrisation input;  // empty

        std::string space;
        ASSERT(combined(user, input, defaults)->get(key, space));

        EXPECT_NOT(defaultSpace.empty());
        EXPECT_EQUAL(space, defaultSpace);
    }


    SECTION("vector-space field-set") {
        const param::SimpleParametrisation user;  // empty

        for (auto& f : tests) {
            param::SimpleParametrisation input;
            input.set(key, f.first);

            std::string space;
            ASSERT(combined(user, input, defaults)->get(key, space));

            EXPECT_NOT(space.empty());

            const auto& sp = data::SpaceChooser::lookup(space);
            Log::info() << "field-set: " << f.first << " dimensions = " << sp.dimensions()
                        << ", expected = " << f.second << std::endl;

            EXPECT(sp.dimensions() == f.second);
        }
    }


    SECTION("vector-space user- and field-set") {
        for (auto& f : tests) {
            param::SimpleParametrisation input;
            input.set(key, f.first);

            for (auto& u : tests) {
                Log::info() << "user-set: " << u.first << ", field-set: " << u.first << "...";

                param::SimpleParametrisation user;
                user.set(key, u.first);

                std::string space;
                ASSERT(combined(user, input, defaults)->get(key, space));

                EXPECT_NOT(space.empty());

                const auto& sp = data::SpaceChooser::lookup(space);
                Log::info() << "user-set: " << u.first << " dimensions = " << sp.dimensions()
                            << ", expected = " << u.second << std::endl;

                EXPECT(sp.dimensions() == u.second);
            }
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
