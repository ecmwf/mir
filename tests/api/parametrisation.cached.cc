// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "mir/param/CachedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"

#include "parametrisation.h"


namespace mir::api::tests {


CASE("CachedParametrisation") {
    SimpleParametrisation underlying;
    underlying.set("a", 1L).set("s", "x").set("vd", std::vector<double>{1., 2.});

    param::CachedParametrisation cached(underlying);
    const MIRParametrisation& p = cached;

    long l = 0;
    EXPECT(p.get("a", l) && l == 1);
    EXPECT(p.has("a"));
    EXPECT_NOT(p.has("b"));

    // the cache keeps a value if the underlying parametrisation changes
    underlying.set("a", 2L);
    EXPECT(p.get("a", l) && l == 1);

    cached.reset();
    EXPECT(p.get("a", l) && l == 2);

    // values set in the cache take precedence
    cached.set("a", 3L);
    cached.set("b", true);
    cached.set("i", 4);
    cached.set("d", 5.);
    cached.set("s", std::string("y"));
    cached.set("c", "z");

    EXPECT(p.get("a", l) && l == 3);
    EXPECT(p.has("b"));

    bool b = false;
    EXPECT(p.get("b", b) && b);

    int i = 0;
    EXPECT(p.get("a", i) && i == 3);

    float f = 0;
    EXPECT_NOT(p.get("x", f));

    double d = 0;
    EXPECT(p.get("d", d) && d == 5.);

    std::string s;
    EXPECT(p.get("s", s) && s == "y");
    EXPECT(p.get("c", s) && s == "z");

    std::vector<int> vi;
    EXPECT_NOT(p.get("x", vi));

    std::vector<long> vl;
    EXPECT_NOT(p.get("x", vl));

    std::vector<float> vf;
    EXPECT_NOT(p.get("x", vf));

    std::vector<double> vd;
    EXPECT(p.get("vd", vd) && vd.size() == 2);

    std::vector<std::string> vs;
    EXPECT_THROWS_AS(p.get("x", vs), eckit::NotImplemented);

    EXPECT(to_string(p).find("CachedParametrisation[") == 0);
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
