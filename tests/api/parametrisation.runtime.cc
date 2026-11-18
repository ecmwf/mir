// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "mir/param/RuntimeParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"

#include "parametrisation.h"


namespace mir::api::tests {


CASE("RuntimeParametrisation") {
    SimpleParametrisation owner;
    owner.set("a", 1L).set("hidden", 1L);

    param::RuntimeParametrisation runtime(owner);
    const MIRParametrisation& p = runtime;

    runtime.set("s", std::string("x"));
    runtime.set("c", "y");
    runtime.set("f", 1.5F);
    runtime.set("d", 2.5);
    runtime.set("i", 3);
    runtime.set("l", 4L);
    runtime.set("ll", 5LL);
    runtime.set("b", true);
    runtime.set("z", size_t(6));
    runtime.set("vi", std::vector<int>{1});
    runtime.set("vl", std::vector<long>{2});
    runtime.set("vll", std::vector<long long>{3});
    runtime.set("vz", std::vector<size_t>{4});
    runtime.set("vf", std::vector<float>{5.F});
    runtime.set("vd", std::vector<double>{6.});
    runtime.set("vs", std::vector<std::string>{"7"});

    // values set at runtime, or from the owner
    long l = 0;
    EXPECT(p.get("a", l) && l == 1);
    EXPECT(p.get("l", l) && l == 4);
    EXPECT(p.get("ll", l) && l == 5);
    EXPECT(p.get("z", l) && l == 6);
    EXPECT(p.has("a") && p.has("l"));

    runtime.set("a", 10L);
    EXPECT(p.get("a", l) && l == 10);

    // hidden values (also from the owner)
    runtime.unset("hidden");
    EXPECT_NOT(p.has("hidden"));
    EXPECT_NOT(p.get("hidden", l));

    bool b = false;
    EXPECT(p.get("b", b) && b);

    int i = 0;
    EXPECT(p.get("l", i) && i == 4);

    double d = 0;
    EXPECT(p.get("d", d) && d == 2.5);

    float f = 0;
    EXPECT_NOT(p.get("missing", f));

    std::string s;
    EXPECT(p.get("s", s) && s == "x");
    EXPECT(p.get("c", s) && s == "y");

    std::vector<int> vi;
    EXPECT_NOT(p.get("missing", vi));

    std::vector<long> vl;
    EXPECT(p.get("vll", vl) && vl == std::vector<long>{3});

    std::vector<float> vf;
    EXPECT_NOT(p.get("missing", vf));

    std::vector<double> vd;
    EXPECT(p.get("vd", vd) && vd == std::vector<double>{6.});

    std::vector<std::string> vs;
    EXPECT_THROWS_AS(p.get("vs", vs), eckit::NotImplemented);

    EXPECT(&p.userParametrisation() == &p);
    EXPECT(&p.fieldParametrisation() == &p);
    EXPECT(to_string(p).find("RuntimeParametrisation[") == 0);
    EXPECT(to_string(p).find(",owner=[") != std::string::npos);
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
