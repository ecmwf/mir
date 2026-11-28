// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "mir/param/SameParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"

#include "parametrisation.h"


namespace mir::api::tests {


CASE("SameParametrisation") {
    SimpleParametrisation p1;
    SimpleParametrisation p2;

    p1.set("same", 1L).set("different", 1L).set("only1", 1L).set("s", "x").set("vd", std::vector<double>{1.});
    p2.set("same", 1L).set("different", 2L).set("only2", 2L).set("s", "x").set("vd", std::vector<double>{1.});

    for (const auto& strict : {true, false}) {
        const param::SameParametrisation same(p1, p2, strict);
        const MIRParametrisation& p = same;

        long l = 0;
        EXPECT(p.get("same", l) && l == 1);
        EXPECT_NOT(p.get("different", l));
        EXPECT_NOT(p.get("missing", l));

        // non-strict: a value in one only is accepted
        EXPECT(p.get("only1", l) == !strict);
        EXPECT(p.get("only2", l) == !strict);

        EXPECT(p.has("only1") && p.has("only2"));
        EXPECT_NOT(p.has("missing"));

        bool b   = false;
        int i    = 0;
        float f  = 0;
        double d = 0;
        std::vector<int> vi;
        std::vector<long> vl;
        std::vector<float> vf;
        std::vector<double> vd;
        std::string s;

        EXPECT_NOT(p.get("missing", b));
        EXPECT(p.get("same", i) && i == 1);
        EXPECT_NOT(p.get("missing", f));
        EXPECT_NOT(p.get("missing", d));
        EXPECT_NOT(p.get("missing", vi));
        EXPECT_NOT(p.get("missing", vl));
        EXPECT_NOT(p.get("missing", vf));
        EXPECT(p.get("vd", vd) && vd == std::vector<double>{1.});
        EXPECT(p.get("s", s) && s == "x");

        std::vector<std::string> vs;
        EXPECT_THROWS_AS(p.get("s", vs), eckit::NotImplemented);

        EXPECT(to_string(p).find("SameParametrisation[parametrisation1=") == 0);
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
