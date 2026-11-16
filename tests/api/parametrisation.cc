// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/JSON.h"
#include "eckit/testing/Test.h"

#include "mir/param/CachedParametrisation.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/param/SameParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/param/SpecParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::api::tests {


using param::MIRParametrisation;
using param::SimpleParametrisation;


template <typename T>
std::string to_string(const T& p) {
    std::ostringstream str;
    str << p;
    return str.str();
}


std::string to_tool_string(const MIRParametrisation& p) {
    std::ostringstream str;
    str << eckit::setformat(Log::applicationFormat) << p;
    return str.str();
}


bool same(double a, double b) {
    return std::abs(a - b) < 1e-9;
}


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


CASE("SpecParametrisation") {
    SECTION("parse") {
        EXPECT(param::SpecParametrisation::parse("", "key").empty());
        EXPECT(param::SpecParametrisation::parse("  ", "key").empty());

        SimpleParametrisation name;
        param::SpecParametrisation::parse(" linear ", "interpolation").set(name);
        EXPECT_EQUAL(to_string(name), R"({"interpolation":"linear"})");

        SimpleParametrisation spec;
        param::SpecParametrisation::parse("{type: k-nearest, nclosest: 8, distance_tolerance: 2}", "interpolation")
            .set(spec);
        EXPECT_EQUAL(to_string(spec), R"({"distance-tolerance":2,"interpolation":"k-nearest","nclosest":8})");
    }


    SECTION("as user parametrisation, combined with the defaults") {
        const param::SpecParametrisation spec("{type: k-nearest, nclosest: 8}", "interpolation");
        const MIRParametrisation& p = spec;

        std::string interpolation;
        EXPECT(p.get("interpolation", interpolation) && interpolation == "k-nearest");

        long nclosest = 0;
        EXPECT(p.get("nclosest", nclosest) && nclosest == 8);

        double distance = 0;
        EXPECT(p.get("distance", distance) && distance == 1.);

        EXPECT(p.userParametrisation().has("nclosest"));
        EXPECT_NOT(p.userParametrisation().has("distance"));
    }
}


CASE("GridSpecParametrisation") {
    struct test_t {
        std::string gridspec;
        std::string gridType;
        std::string grid;
    };


    SECTION("gridType and grid") {
        for (const auto& test : std::vector<test_t>{
                 {"{grid: F16}", "regular_gg", "F16"},
                 {"{grid: N16}", "reduced_gg", "N16"},
                 {"{grid: O16}", "reduced_gg", "O16"},
                 {"{grid: O16, rotation: [-40, 22]}", "reduced_rotated_gg", "O16"},
                 {"{grid: F16, rotation: [-40, 22]}", "rotated_gg", "F16"},
                 {"{grid: H4}", "healpix", "H4"},
                 {"{grid: H4, order: nested}", "healpix", "HN4"},
                 {"{grid: eORCA1_T}", "unstructured_grid", "eORCA1_T"},
             }) {
            const param::GridSpecParametrisation p(test.gridspec);
            const MIRParametrisation& m = p;

            std::string gridType;
            EXPECT(m.get("gridType", gridType));
            EXPECT_EQUAL(gridType, test.gridType);

            std::string grid;
            EXPECT(m.get("grid", grid));
            EXPECT_EQUAL(grid, test.grid);

            bool gridded = false;
            EXPECT(m.get("gridded", gridded) && gridded);
            EXPECT_NOT(m.has("spectral"));
            EXPECT_NOT(m.has("gridded_regular_ll"));

            std::string gridSpec;
            EXPECT(m.get("gridSpec", gridSpec) && !gridSpec.empty());

            EXPECT_EQUAL(to_string(m), "GridSpecParametrisation[" + gridSpec + "]");
        }
    }


    SECTION("regular_ll") {
        const param::GridSpecParametrisation p("{grid: [2, 1], area: [10, 0, -10, 20]}");
        const MIRParametrisation& m = p;

        std::string gridType;
        EXPECT(m.get("gridType", gridType) && gridType == "regular_ll");

        bool b = false;
        EXPECT(m.get("gridded_regular_ll", b) && b);

        std::vector<double> grid;
        EXPECT(m.get("grid", grid) && grid == std::vector<double>({2., 1.}));

        long Ni = 0;
        long Nj = 0;
        EXPECT(m.get("Ni", Ni) && Ni == 11);
        EXPECT(m.get("Nj", Nj) && Nj == 21);

        double north = 0;
        double south = 0;
        double west  = 0;
        double east  = 0;
        double we    = 0;
        double sn    = 0;
        EXPECT(m.get("north", north) && north == 10.);
        EXPECT(m.get("south", south) && south == -10.);
        EXPECT(m.get("west", west) && west == 0.);
        EXPECT(m.get("east", east) && east == 20.);
        EXPECT(m.get("west_east_increment", we) && we == 2.);
        EXPECT(m.get("south_north_increment", sn) && sn == 1.);

        std::string order;
        EXPECT(m.get("order", order) && !order.empty());

        EXPECT_NOT(m.has("gridName"));
        EXPECT_NOT(m.has("pl"));
        EXPECT_NOT(m.has("N"));
        EXPECT_NOT(m.has("rotation"));
        EXPECT_NOT(m.has("truncation"));
        EXPECT_NOT(m.has("uid"));
        EXPECT_NOT(m.has("latitudes/longitudes"));
        EXPECT_NOT(m.has("unknown"));

        // getters of other types
        int i = 0;
        EXPECT(m.get("Ni", i) && i == 11);

        float f = 0;
        EXPECT_NOT(m.get("unknown", f));

        std::vector<int> vi;
        EXPECT_NOT(m.get("unknown", vi));

        std::vector<long> vl;
        EXPECT_NOT(m.get("unknown", vl));

        std::vector<float> vf;
        EXPECT_NOT(m.get("unknown", vf));

        std::vector<std::string> vs;
        EXPECT_THROWS_AS(m.get("gridType", vs), eckit::NotImplemented);
    }


    SECTION("rotated regular_ll") {
        const param::GridSpecParametrisation p("{grid: [1, 1], area: [10, -10, -10, 10], rotation: [-40, 22]}");
        const MIRParametrisation& m = p;

        std::string gridType;
        EXPECT(m.get("gridType", gridType) && gridType == "rotated_ll");

        double lat   = 0;
        double lon   = 0;
        double angle = 1;
        EXPECT(m.get("south_pole_latitude", lat) && lat == -40.);
        EXPECT(m.get("south_pole_longitude", lon) && lon == 22.);
        EXPECT(m.get("south_pole_rotation_angle", angle) && angle == 0.);

        std::vector<double> rotation;
        EXPECT(m.get("rotation", rotation) && rotation == std::vector<double>({-40., 22.}));
    }


    SECTION("regular_gg") {
        const param::GridSpecParametrisation p("{grid: F16}");
        const MIRParametrisation& m = p;

        long N = 0;
        EXPECT(m.get("N", N) && N == 16);
        EXPECT(m.get("regular", N) && N == 16);

        long Ni = 0;
        long Nj = 0;
        EXPECT(m.get("Ni", Ni) && Ni == 64);
        EXPECT(m.get("Nj", Nj) && Nj == 32);

        double we = 0;
        EXPECT(m.get("west_east_increment", we) && we == 360. / 64.);
        EXPECT_NOT(m.has("south_north_increment"));

        std::string gridName;
        EXPECT(m.get("gridName", gridName) && gridName == "F16");
    }


    SECTION("reduced_gg") {
        for (const auto& test :
             std::vector<test_t>{{"{grid: O16}", "octahedral", "O16"}, {"{grid: N16}", "reduced", "N16"}}) {
            const param::GridSpecParametrisation p(test.gridspec);
            const MIRParametrisation& m = p;

            long N = 0;
            EXPECT(m.get("N", N) && N == 16);
            EXPECT(m.get(test.gridType, N) && N == 16);

            std::vector<long> pl;
            EXPECT(m.get("pl", pl) && pl.size() == 32);

            long Nj = 0;
            EXPECT(m.get("Nj", Nj) && Nj == 32);

            double north = 0;
            double south = 0;
            double west  = 1;
            double east  = 0;
            EXPECT(m.get("north", north) && 0. < north && north < 90.);
            EXPECT(m.get("south", south) && same(south, -north));
            EXPECT(m.get("west", west) && west == 0.);
            EXPECT(m.get("east", east) && 350. < east && east < 360.);

            std::string order;
            EXPECT(m.get("order", order) && !order.empty());
        }


        // a regional grid keeps all latitudes, those without points have no longitudes
        const param::GridSpecParametrisation p("{grid: O16, area: [60, -10, 30, 40]}");
        const MIRParametrisation& m = p;

        double north = 0;
        double south = 0;
        double west  = 0;
        double east  = 0;
        long Nj      = 0;
        EXPECT(m.get("north", north) && north <= 60. && north > 50.);
        EXPECT(m.get("south", south) && south >= 30. && south < 40.);
        EXPECT(m.get("west", west) && west >= -10. && west < 0.);
        EXPECT(m.get("east", east) && east <= 40. && east > 30.);
        EXPECT(m.get("Nj", Nj) && 0 < Nj && Nj < 32);
    }


    SECTION("reduced_ll") {
        const param::GridSpecParametrisation p("{type: reduced_ll, pl: [4, 8, 8, 4]}");
        const MIRParametrisation& m = p;

        std::string gridType;
        EXPECT(m.get("gridType", gridType) && gridType == "reduced_ll");

        std::vector<long> pl;
        EXPECT(m.get("pl", pl) && pl == std::vector<long>({4, 8, 8, 4}));
    }


    SECTION("HEALPix") {
        for (const auto& order : std::vector<std::string>{"ring", "nested"}) {
            const param::GridSpecParametrisation p("{grid: H4, order: " + order + "}");
            const MIRParametrisation& m = p;

            long Nside = 0;
            EXPECT(m.get("Nside", Nside) && Nside == 4);

            std::string ordering;
            EXPECT(m.get("orderingConvention", ordering) && ordering == order);

            std::string gridName;
            EXPECT(m.get("gridName", gridName) && gridName == "H4");

            double west = 0;
            EXPECT(m.get("west", west) && west == 45.);
            EXPECT(m.get("longitudeOfFirstGridPointInDegrees", west) && west == 45.);

            EXPECT_NOT(m.has("pl"));
        }
    }


    SECTION("spherical harmonics") {
        const param::GridSpecParametrisation p("{grid: T21}");
        const MIRParametrisation& m = p;

        std::string gridType;
        EXPECT(m.get("gridType", gridType) && gridType == "sh");

        bool spectral = false;
        EXPECT(m.get("spectral", spectral) && spectral);
        EXPECT_NOT(m.has("gridded"));
        EXPECT_NOT(m.has("grid"));

        long truncation = 0;
        EXPECT(m.get("truncation", truncation) && truncation == 21);
    }


    SECTION("unstructured") {
        const param::GridSpecParametrisation orca("{grid: eORCA1_T}");

        std::string uid;
        EXPECT(static_cast<const MIRParametrisation&>(orca).get("uid", uid) && uid.size() == 32);
        EXPECT_NOT(static_cast<const MIRParametrisation&>(orca).has("latitudes/longitudes"));

        const param::GridSpecParametrisation p(
            "{type: unstructured_ll, latitudes: [0, 10, 20], longitudes: [0, 10, 20]}");
        const MIRParametrisation& m = p;

        std::string gridType;
        EXPECT(m.get("gridType", gridType) && gridType == "unstructured_ll");
        EXPECT(m.has("latitudes/longitudes"));

        std::vector<double> lats;
        std::vector<double> lons;
        EXPECT(m.get("latitudes", lats) && lats == std::vector<double>({0., 10., 20.}));
        EXPECT(m.get("longitudes", lons) && lons == std::vector<double>({0., 10., 20.}));
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
