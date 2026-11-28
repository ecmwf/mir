// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/JSON.h"
#include "eckit/testing/Test.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"

#include "parametrisation.h"


namespace mir::api::tests {


CASE("SimpleParametrisation: set, get and conversions") {
    SimpleParametrisation p;


    SECTION("bool") {
        p.set("b", true);

        bool b = false;
        EXPECT(p.get("b", b) && b);

        std::string s;
        EXPECT(p.get("b", s) && s == "true");

        long l = 0;
        EXPECT_THROWS_AS(p.get("b", l), exception::CannotConvert);
    }


    SECTION("int, long, long long, size_t") {
        p.set("i", 1).set("l", 2L).set("ll", 3LL).set("z", size_t(4));

        long l = 0;
        EXPECT(p.get("i", l) && l == 1);
        EXPECT(p.get("l", l) && l == 2);
        EXPECT(p.get("ll", l) && l == 3);
        EXPECT(p.get("z", l) && l == 4);

        int i = 0;
        EXPECT(p.get("l", i) && i == 2);

        // integers as reals
        double d = 0;
        EXPECT(p.get("i", d) && d == 1.);
        EXPECT(p.get("l", d) && d == 2.);

        std::string s;
        EXPECT(p.get("i", s) && s == "1");
        EXPECT(p.get("l", s) && s == "2");

        bool b = false;
        EXPECT_THROWS_AS(p.get("l", b), exception::CannotConvert);
        EXPECT_THROWS_AS(p.get("i", b), exception::CannotConvert);

        float f = 0;
        EXPECT_THROWS_AS(p.get("l", f), exception::CannotConvert);
    }


    SECTION("float, double") {
        p.set("f", 1.5F).set("d", 2.5);

        double d = 0;
        EXPECT(p.get("d", d) && d == 2.5);

        std::string s;
        EXPECT(p.get("f", s) && s == "1.500000");
        EXPECT(p.get("d", s) && s == "2.500000");

        long l = 0;
        EXPECT_THROWS_AS(p.get("d", l), exception::CannotConvert);
        EXPECT_THROWS_AS(p.get("f", d), exception::CannotConvert);
    }


    SECTION("string") {
        p.set("s", std::string("1")).set("c", "2.5").set("v", "1/2/3").set("t", "true");

        std::string s;
        EXPECT(p.get("s", s) && s == "1");

        bool b = false;
        EXPECT(p.get("t", b) && b);

        int i = 0;
        EXPECT(p.get("s", i) && i == 1);

        long l = 0;
        EXPECT(p.get("s", l) && l == 1);

        size_t z = 0;
        EXPECT(static_cast<const MIRParametrisation&>(p).get("s", z) && z == 1);

        float f = 0;
        EXPECT(p.get("c", f) && f == 2.5F);

        double d = 0;
        EXPECT(p.get("c", d) && d == 2.5);

        std::vector<double> v;
        EXPECT(p.get("v", v) && v == std::vector<double>({1., 2., 3.}));

        std::vector<long> vl;
        EXPECT_THROWS_AS(p.get("v", vl), exception::CannotConvert);
    }


    SECTION("vectors") {
        p.set("vb", std::vector<bool>{true, false, true})
            .set("vi", std::vector<int>{1, 2})
            .set("vl", std::vector<long>{3, 4})
            .set("vll", std::vector<long long>{5, 6})
            .set("vz", std::vector<size_t>{7, 8})
            .set("vf", std::vector<float>{1.5F})
            .set("vd", std::vector<double>{1.5, 2.5})
            .set("vs", std::vector<std::string>{"a", "b"});

        std::vector<long> vl;
        EXPECT(p.get("vb", vl) && vl == std::vector<long>({1, 0, 1}));
        EXPECT(p.get("vl", vl) && vl == std::vector<long>({3, 4}));
        EXPECT(p.get("vll", vl) && vl == std::vector<long>({5, 6}));

        std::vector<double> vd;
        EXPECT(p.get("vl", vd) && vd == std::vector<double>({3., 4.}));
        EXPECT(p.get("vd", vd) && vd == std::vector<double>({1.5, 2.5}));

        std::string s;
        EXPECT(p.get("vi", s) && s == "1/2");
        EXPECT(p.get("vl", s) && s == "3/4");
        EXPECT(p.get("vz", s) && s == "7/8");
        EXPECT(p.get("vf", s) && s == "1.500000");
        EXPECT(p.get("vd", s) && s == "1.500000/2.500000");
        EXPECT(p.get("vs", s) && s == "a/b");

        std::vector<int> vi;
        EXPECT_THROWS_AS(p.get("vi", vi), exception::CannotConvert);

        std::vector<float> vf;
        EXPECT_THROWS_AS(p.get("vf", vf), exception::CannotConvert);

        long l = 0;
        EXPECT_THROWS_AS(p.get("vl", l), exception::CannotConvert);

        std::vector<std::string> vs;
        EXPECT_THROWS_AS(p.get("vs", vs), eckit::NotImplemented);
    }


    SECTION("MIRParametrisation conversions (long long, size_t and vectors)") {
        p.set("l", 2L).set("vl", std::vector<long>{0, 1, 2});
        const MIRParametrisation& m = p;

        long long ll = 0;
        EXPECT(m.get("l", ll) && ll == 2);

        size_t z = 0;
        EXPECT(m.get("l", z) && z == 2);

        std::vector<bool> vb;
        EXPECT(m.get("vl", vb) && vb == std::vector<bool>({false, true, true}));

        std::vector<long long> vll;
        EXPECT(m.get("vl", vll) && vll == std::vector<long long>({0, 1, 2}));

        std::vector<size_t> vz;
        EXPECT(m.get("vl", vz) && vz == std::vector<size_t>({0, 1, 2}));

        // missing keys
        EXPECT_NOT(m.get("missing", ll));
        EXPECT_NOT(m.get("missing", z));
        EXPECT_NOT(m.get("missing", vb));
        EXPECT_NOT(m.get("missing", vll));
        EXPECT_NOT(m.get("missing", vz));

        // no user/field parametrisations
        EXPECT_THROWS_AS(m.userParametrisation(), exception::SeriousBug);
        EXPECT_THROWS_AS(m.fieldParametrisation(), exception::SeriousBug);
    }


    SECTION("missing keys") {
        bool b   = false;
        int i    = 0;
        long l   = 0;
        float f  = 0;
        double d = 0;
        std::string s;
        std::vector<int> vi;
        std::vector<long> vl;
        std::vector<float> vf;
        std::vector<double> vd;

        EXPECT_NOT(p.has("x"));
        EXPECT_NOT(p.get("x", b));
        EXPECT_NOT(p.get("x", i));
        EXPECT_NOT(p.get("x", l));
        EXPECT_NOT(p.get("x", f));
        EXPECT_NOT(p.get("x", d));
        EXPECT_NOT(p.get("x", s));
        EXPECT_NOT(p.get("x", vi));
        EXPECT_NOT(p.get("x", vl));
        EXPECT_NOT(p.get("x", vf));
        EXPECT_NOT(p.get("x", vd));
    }
}


CASE("SimpleParametrisation: management, print and json") {
    SimpleParametrisation p;
    p.set("a", 1L).set("b", "x").set("c", std::vector<double>{1., 2.}).set("d", std::vector<long>{3, 4});
    EXPECT_EQUAL(p.size(), 4);

    // replacing a value
    p.set("a", 2L);
    EXPECT_EQUAL(p.size(), 4);

    EXPECT_EQUAL(to_string(p), R"({"a":2,"b":"x","c":[1,2],"d":[3,4]})");
    EXPECT_EQUAL(to_tool_string(p), "--a=2 --b=x --c=1/2 --d=3/4");

    std::ostringstream json;
    eckit::JSON j(json);
    j << p;
    EXPECT_EQUAL(json.str(), R"({"a":2,"b":"x","c":[1,2],"d":[3,4]})");


    SECTION("copyValuesTo, swap") {
        SimpleParametrisation q;
        q.set("e", true);
        p.copyValuesTo(q);
        EXPECT_EQUAL(q.size(), 5);
        EXPECT(p.matchAll(q));

        SimpleParametrisation r;
        r.swap(q);
        EXPECT_EQUAL(q.size(), 0);
        EXPECT_EQUAL(r.size(), 5);
    }


    SECTION("clear, reset") {
        p.clear("a").clear("does-not-exist");
        EXPECT_NOT(p.has("a"));
        EXPECT_EQUAL(p.size(), 3);

        p.reset();
        EXPECT_EQUAL(p.size(), 0);
        EXPECT_EQUAL(to_string(p), "{}");
    }
}


CASE("SimpleParametrisation: matchAll, matchAny") {
    SimpleParametrisation other;
    other.set("paramId", 167L)
        .set("grid", std::vector<double>{1., 1.})
        .set("west_east_increment", 1.)
        .set("gridType", "regular_ll")
        .set("level", 1000L);


    SECTION("scalars") {
        SimpleParametrisation rule;
        rule.set("paramId", 167L).set("gridType", "regular_ll");
        EXPECT(rule.matchAll(other));
        EXPECT(rule.matchAny(other));

        rule.set("gridType", "reduced_gg");
        EXPECT_NOT(rule.matchAll(other));
        EXPECT_NOT(rule.matchAny(other));

        SimpleParametrisation empty;
        EXPECT(empty.matchAll(other));
        EXPECT(empty.matchAny(other));
    }


    SECTION("vector<long>") {
        SimpleParametrisation rule;

        // all values match, or any value matches
        rule.set("paramId", std::vector<long>{167, 168});
        EXPECT_NOT(rule.matchAll(other));
        EXPECT(rule.matchAny(other));

        rule.set("paramId", std::vector<long>{1, 2});
        EXPECT_NOT(rule.matchAny(other));

        rule.set("missing", std::vector<long>{1});
        EXPECT_NOT(rule.matchAll(other));
        EXPECT_NOT(rule.matchAny(other));
    }


    SECTION("vector<double>") {
        SimpleParametrisation all;
        all.set("grid", std::vector<double>{1., 1.});
        EXPECT(all.matchAll(other));

        all.set("grid", std::vector<double>{1., 2.});
        EXPECT_NOT(all.matchAll(other));

        SimpleParametrisation any;
        any.set("west_east_increment", std::vector<double>{0.5, 1.});
        EXPECT(any.matchAny(other));

        any.set("west_east_increment", std::vector<double>{0.5, 2.});
        EXPECT_NOT(any.matchAny(other));

        // a value of another type does not match
        any.set("gridType", std::vector<double>{1.});
        EXPECT_NOT(any.matchAny(other));
        EXPECT_NOT(any.matchAll(other));
    }


    SECTION("vector<string>") {
        SimpleParametrisation any;
        any.set("gridType", std::vector<std::string>{"reduced_gg", "regular_ll"});
        EXPECT(any.matchAny(other));

        any.set("gridType", std::vector<std::string>{"reduced_gg"});
        EXPECT_NOT(any.matchAny(other));

        // other.get(vector<string>) is not implemented
        EXPECT_NOT(any.matchAll(other));
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
