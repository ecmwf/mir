// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/filesystem/TmpDir.h"
#include "eckit/testing/Test.h"

#include "mir/action/plan/Action.h"
#include "mir/api/MIRComplexJob.h"
#include "mir/api/MIRJob.h"
#include "mir/api/MIRWatcher.h"
#include "mir/input/GridSpecInput.h"
#include "mir/input/RawInput.h"
#include "mir/key/grid/Grid.h"
#include "mir/output/EmptyOutput.h"
#include "mir/output/ValuesOutput.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


namespace mir::api::tests {


/// Input of a field on a grid (by its gridSpec), values as given
struct GridInput {
    explicit GridInput(const std::string& gridspec, double value = 1.) : param(gridspec) {
        values.assign(param.grid().size(), value);
        input = std::make_unique<input::RawInput>(values.data(), values.size(), param);
    }

    param::GridSpecParametrisation param;
    std::vector<double> values;
    std::unique_ptr<input::MIRInput> input;
};


std::string read_file(const std::string& path) {
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}


CASE("Representation::fillJob(MIRJob&)") {
    std::vector<std::string> tests{
        "F16", "O16", "F21", "O21", "N16", "eORCA1_T",
    };

    for (const auto& test : tests) {
        repres::RepresentationHandle repres = key::grid::Grid::lookup(test).representation();
        ASSERT(repres);

        MIRJob job;
        repres->fillJob(job);

        std::string grid;
        EXPECT(job.get("grid", grid));
        EXPECT(grid == R"({"grid":")" + test + R"("})");
    }
}


CASE("MIRJob: set (typed values)") {
    MIRJob job;

    SECTION("scalars") {
        job.set("a-float", 1.5F)
            .set("a-double", 2.5)
            .set("an-int", 3)
            .set("a-long", 4L)
            .set("a-long-long", 5LL)
            .set("a-bool", true)
            .set("a-size_t", size_t(6))
            .set("a-string", std::string("seven"))
            .set("a-char", "eight");

        double d = 0;
        EXPECT(job.get("a-double", d) && d == 2.5);

        long l = 0;
        EXPECT(job.get("an-int", l) && l == 3);
        EXPECT(job.get("a-long", l) && l == 4);
        EXPECT(job.get("a-long-long", l) && l == 5);
        EXPECT(job.get("a-size_t", l) && l == 6);

        bool b = false;
        EXPECT(job.get("a-bool", b) && b);

        std::string s;
        EXPECT(job.get("a-string", s) && s == "seven");
        EXPECT(job.get("a-char", s) && s == "eight");
        EXPECT(job.get("a-float", s) && s.find("1.5") == 0);

        EXPECT_EQUAL(job.size(), 9);
    }


    SECTION("vectors") {
        job.set("v-bool", std::vector<bool>{true, false})
            .set("v-int", std::vector<int>{1, 2})
            .set("v-long", std::vector<long>{3, 4})
            .set("v-long-long", std::vector<long long>{5, 6})
            .set("v-size_t", std::vector<size_t>{7, 8})
            .set("v-float", std::vector<float>{9.F})
            .set("v-double", std::vector<double>{1., 2., 3., 4., 5., 6.})
            .set("v-string", std::vector<std::string>{"a", "b"});

        std::vector<long> l;
        EXPECT(job.get("v-bool", l) && l == std::vector<long>({1, 0}));
        EXPECT(job.get("v-long", l) && l == std::vector<long>({3, 4}));
        EXPECT(job.get("v-long-long", l) && l == std::vector<long>({5, 6}));

        std::vector<double> d;
        EXPECT(job.get("v-double", d) && d.size() == 6);

        std::string s;
        EXPECT(job.get("v-int", s) && s == "1/2");
        EXPECT(job.get("v-size_t", s) && s == "7/8");
        EXPECT(job.get("v-string", s) && s == "a/b");
    }


    SECTION("pairs and quadruples") {
        job.set("grid", 1., 2.).set("area", 10., 20., 30., 40.);

        std::vector<double> grid;
        EXPECT(job.get("grid", grid) && grid == std::vector<double>({1., 2.}));

        std::vector<double> area;
        EXPECT(job.get("area", area) && area == std::vector<double>({10., 20., 30., 40.}));
    }


    SECTION("aliases") {
        job.set("gridname", "O16");

        std::string grid;
        EXPECT(job.get("grid", grid) && grid == "O16");
        EXPECT_NOT(job.has("gridname"));

        job.clear("gridname");
        EXPECT_NOT(job.has("grid"));
    }
}


CASE("MIRJob: set (command-line arguments)") {
    MIRJob job;
    job.set("--grid=1/1 --area=10/0/0/10 --caching=false --dryrun");

    std::vector<double> grid;
    EXPECT(job.get("grid", grid) && grid == std::vector<double>({1., 1.}));

    bool caching = true;
    EXPECT(job.get("caching", caching) && !caching);

    bool dryrun = false;
    EXPECT(job.get("dryrun", dryrun) && dryrun);

    EXPECT_THROWS_AS(job.set("grid=1/1"), exception::UserError);
}


CASE("MIRJob: interpolation, by name or by spec") {
    // by name, its options set separately
    MIRJob a;
    a.set("interpolation", "k-nearest");
    a.set("nclosest", 8L);

    // by spec, setting its options
    MIRJob b;
    b.set("interpolation", "{type: k-nearest, nclosest: 8}");

    for (const auto* job : {&a, &b}) {
        std::string interpolation;
        long nclosest = 0;
        EXPECT(job->get("interpolation", interpolation) && interpolation == "k-nearest");
        EXPECT(job->get("nclosest", nclosest) && nclosest == 8);
    }

    // interpolation options are set one way, by spec or by option
    EXPECT_THROWS_AS(b.set("nclosest", 4L), exception::UserError);
    EXPECT_THROWS_AS(a.set("interpolation", "{type: k-nearest, nclosest: 4}"), exception::UserError);

    // another interpolation replaces the options of the previous spec
    b.set("interpolation", "linear");
    EXPECT_NOT(b.has("nclosest"));
    b.set("nclosest", 4L);

    b.clear("nclosest");
    b.set("interpolation", "{type: k-nearest, nclosest: 8}");
    b.clear("interpolation");
    EXPECT_NOT(b.has("interpolation"));
    EXPECT_NOT(b.has("nclosest"));
}


CASE("MIRJob: print, mirToolCall, json") {
    MIRJob job;
    job.set("grid", 1., 1.).set("caching", false);

    std::ostringstream str;
    str << job;
    EXPECT_EQUAL(str.str(), R"(MIRJob[{"caching":false,"grid":[1,1]}])");

    std::ostringstream call;
    job.mirToolCall(call);
    EXPECT_EQUAL(call.str(), "mir--caching=0 --grid=1/1 in.grib out.grib");

    EXPECT_EQUAL(job.json_str(), R"({"caching":false,"grid":[1,1]})");

    // the stream format is restored
    std::ostringstream after;
    job.mirToolCall(after);
    after << job;
    EXPECT(after.str().find("MIRJob[") != std::string::npos);
}


CASE("MIRJob: representationFrom") {
    std::unique_ptr<input::MIRInput> input(new input::GridSpecInput("{grid: O16}"));
    EXPECT(input->next());

    MIRJob job;
    job.representationFrom(*input);

    std::string grid;
    EXPECT(job.get("grid", grid));
    EXPECT_EQUAL(grid, R"({"grid":"O16"})");
}


CASE("MIRJob: action and grid plans (JSON)") {
    GridInput in("{grid: [10, 10]}");
    ASSERT(in.input->next());

    MIRJob job;
    job.set("grid", 20., 20.).set("caching", false);

    auto actions = job.action_plan_json(*in.input);
    Log::info() << "action plan: " << actions << std::endl;
    EXPECT(actions.find("interpolate.grid2regular-ll") != std::string::npos);

    auto grids = job.grid_plan_json(*in.input);
    Log::info() << "grid plan: " << grids << std::endl;
    EXPECT(grids.find("[10,10]") != std::string::npos);
    EXPECT(grids.find("[20,20]") != std::string::npos);

    // execution is not affected
    output::ValuesOutput out;
    job.execute(*in.input, out);
    EXPECT_EQUAL(out.values().size(), 18 * 9);
}


struct CountingWatcher final : MIRWatcher {
    explicit CountingWatcher(bool rethrow) : rethrow_(rethrow) {}

    bool failure(std::exception&, const action::Action&) override {
        ++failures;
        return rethrow_;
    }

    void print(std::ostream& out) const override { out << "CountingWatcher[]"; }

    size_t failures = 0;

private:
    const bool rethrow_;
};


struct FailingOutput final : output::EmptyOutput {
    size_t save(const param::MIRParametrisation&, context::Context&) override {
        throw exception::WriteError("FailingOutput");
    }
};


CASE("MIRComplexJob") {
    GridInput in("{grid: [10, 10]}", 2.);
    ASSERT(in.input->next());

    util::MIRStatistics statistics;


    SECTION("empty") {
        MIRComplexJob complex;
        EXPECT(complex.empty());
        EXPECT_NO_THROW(complex.execute(statistics));

        output::EmptyOutput out;
        complex.add(nullptr, *in.input, out);
        EXPECT(complex.empty());
    }


    SECTION("jobs sharing an input") {
        output::ValuesOutput out1;
        output::ValuesOutput out2;

        auto* job1 = new MIRJob;
        job1->set("grid", 20., 20.).set("caching", false);

        auto* job2 = new MIRJob;
        job2->set("grid", 30., 30.).set("caching", false);

        MIRComplexJob complex;
        complex.add(job1, *in.input, out1).add(job2, *in.input, out2);
        EXPECT_NOT(complex.empty());

        complex.execute(statistics);

        EXPECT_EQUAL(out1.values().size(), 18 * 9);
        EXPECT_EQUAL(out2.values().size(), 12 * 7);
        for (const auto& v : out1.values()) {
            EXPECT_EQUAL(v, 2.);
        }

        complex.clear();
        EXPECT(complex.empty());
    }


    SECTION("jobs must share the input") {
        GridInput other("{grid: [10, 10]}");
        output::ValuesOutput out;

        MIRComplexJob complex;
        complex.add(new MIRJob, *in.input, out);

        auto* job = new MIRJob;
        EXPECT_THROWS_AS(complex.add(job, *other.input, out), exception::SeriousBug);
        delete job;
    }


    SECTION("dump action and grid plans") {
        eckit::TmpDir dir;
        auto actions = (dir / "actions.json").asString();
        auto grids   = (dir / "grids.json").asString();

        auto* job = new MIRJob;
        job->set("grid", 20., 20.).set("caching", false);
        job->set("dump-action-plan-file", actions).set("dump-grid-plan-file", grids);

        output::ValuesOutput out;

        MIRComplexJob complex;
        complex.add(job, *in.input, out);
        complex.execute(statistics);

        auto a = read_file(actions);
        auto g = read_file(grids);
        Log::info() << "actions: " << a << "\ngrids: " << g << std::endl;

        EXPECT(a.find("interpolate.grid2regular-ll") != std::string::npos);
        EXPECT(g.find("[10,10]") != std::string::npos);
        EXPECT(g.find("[20,20]") != std::string::npos);
    }


    SECTION("watcher") {
        for (const auto& rethrow : {false, true}) {
            FailingOutput out;

            // owned by the complex job
            auto* watcher = new CountingWatcher(rethrow);

            auto* job = new MIRJob;
            job->set("grid", 20., 20.).set("caching", false);

            MIRComplexJob complex;
            complex.add(job, *in.input, out, watcher);

            if (rethrow) {
                EXPECT_THROWS_AS(complex.execute(statistics), exception::WriteError);
            }
            else {
                EXPECT_NO_THROW(complex.execute(statistics));
            }

            EXPECT_EQUAL(watcher->failures, 1);
            std::ostringstream str;
            str << *watcher;
            EXPECT_EQUAL(str.str(), "CountingWatcher[]");
        }
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
