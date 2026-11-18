// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"

#include "mir/param/ConfigurationWrapper.h"

#include "parametrisation.h"


namespace mir::api::tests {


CASE("ConfigurationWrapper") {
    eckit::LocalConfiguration config;
    config.set("b", true);
    config.set("i", 1);
    config.set("l", 2L);
    config.set("d", 3.5);
    config.set("s", "x");
    config.set("vi", std::vector<int>{1, 2});
    config.set("vl", std::vector<long>{3, 4});
    config.set("vd", std::vector<double>{5.5});
    config.set("vs", std::vector<std::string>{"a", "b"});

    const param::ConfigurationWrapper wrapper(config);
    const MIRParametrisation& p = wrapper;

    EXPECT(p.has("b"));
    EXPECT_NOT(p.has("missing"));

    bool b = false;
    EXPECT(p.get("b", b) && b);

    int i = 0;
    EXPECT(p.get("i", i) && i == 1);

    long l = 0;
    EXPECT(p.get("l", l) && l == 2);

    float f = 0;
    EXPECT(p.get("d", f) && f == 3.5F);

    double d = 0;
    EXPECT(p.get("d", d) && d == 3.5);

    std::string s;
    EXPECT(p.get("s", s) && s == "x");

    std::vector<int> vi;
    EXPECT(p.get("vi", vi) && vi == std::vector<int>({1, 2}));

    std::vector<long> vl;
    EXPECT(p.get("vl", vl) && vl == std::vector<long>({3, 4}));

    std::vector<float> vf;
    EXPECT(p.get("vd", vf) && vf == std::vector<float>{5.5F});

    std::vector<double> vd;
    EXPECT(p.get("vd", vd) && vd == std::vector<double>{5.5});

    std::vector<std::string> vs;
    EXPECT(p.get("vs", vs) && vs == std::vector<std::string>({"a", "b"}));

    EXPECT(&p.userParametrisation() == &p);
    EXPECT(&p.fieldParametrisation() == &p);
    EXPECT(to_string(p).find("ConfigurationWrapper[") == 0);
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
