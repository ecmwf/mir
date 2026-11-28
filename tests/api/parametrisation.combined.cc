// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"

#include "parametrisation.h"


namespace mir::api::tests {


CASE("CombinedParametrisation") {
    SimpleParametrisation user;
    SimpleParametrisation field;
    SimpleParametrisation defaults;

    user.set("a", 1L).set("vd", std::vector<double>{1.});
    field.set("a", 2L).set("b", 2L).set("s", "field").set("vl", std::vector<long>{2});
    defaults.set("a", 3L).set("b", 3L).set("c", 3L).set("t", true).set("d", 3.);

    const param::CombinedParametrisation combined(user, field, defaults);
    const MIRParametrisation& p = combined;

    // user, then field, then defaults
    long l = 0;
    EXPECT(p.get("a", l) && l == 1);
    EXPECT(p.get("b", l) && l == 2);
    EXPECT(p.get("c", l) && l == 3);
    EXPECT_NOT(p.get("x", l));

    EXPECT(p.has("a") && p.has("b") && p.has("c"));
    EXPECT_NOT(p.has("x"));

    bool b = false;
    EXPECT(p.get("t", b) && b);

    int i = 0;
    EXPECT(p.get("c", i) && i == 3);

    float f = 0;
    EXPECT_NOT(p.get("x", f));

    double d = 0;
    EXPECT(p.get("d", d) && d == 3.);

    std::string s;
    EXPECT(p.get("s", s) && s == "field");

    std::vector<int> vi;
    EXPECT_NOT(p.get("x", vi));

    std::vector<long> vl;
    EXPECT(p.get("vl", vl) && vl == std::vector<long>{2});

    std::vector<float> vf;
    EXPECT_NOT(p.get("x", vf));

    std::vector<double> vd;
    EXPECT(p.get("vd", vd) && vd == std::vector<double>{1.});

    std::vector<std::string> vs;
    EXPECT_THROWS_AS(p.get("vd", vs), eckit::NotImplemented);

    EXPECT(&p.userParametrisation() == &user);
    EXPECT(&p.fieldParametrisation() == &field);
    EXPECT_EQUAL(to_string(p), "CombinedParametrisation[]");


    SECTION("defaults") {
        SimpleParametrisation empty;
        const param::CombinedParametrisation c(empty);

        std::string interpolation;
        EXPECT(static_cast<const MIRParametrisation&>(c).get("interpolation", interpolation));
        EXPECT_EQUAL(interpolation, "linear");
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
