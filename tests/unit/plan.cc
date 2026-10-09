// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/utils/StringTools.h"

#include "mir/action/plan/Action.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/action/plan/Job.h"
#include "mir/api/MIRJob.h"
#include "mir/input/MIRInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"


namespace mir::tests::unit {


// plan (printed actions), prepared but not executed
std::vector<std::string> plan(const std::string& artificialInput, const api::MIRJob& job) {
    param::SimpleParametrisation field;
    field.set("input", artificialInput);

    std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build("constant", field));
    output::EmptyOutput output;

    const action::Job plan(job, *input, output, false);
    Log::info() << plan.plan() << std::endl;

    std::vector<std::string> actions;
    for (const auto* action : plan.plan()) {
        std::ostringstream str;
        str << *action;
        actions.emplace_back(str.str());
    }
    return actions;
}


bool starts(const std::string& action, const std::string& prefix) {
    return eckit::StringTools::startsWith(action, prefix);
}


bool ends(const std::string& action, const std::string& suffix) {
    return eckit::StringTools::endsWith(action, suffix);
}


bool contains(const std::string& action, const std::string& substring) {
    return action.find(substring) != std::string::npos;
}


const std::string T1279 = "{artificialInput:constant,constant:0.,spectral:true,truncation:1279,gridType:sh}";

// eORCA1_T, with the intermediate grid and interpolation from the grid catalog and grid-type.yaml
const std::string ORCA =
    "{artificialInput:constant,constant:0.,gridded:true,gridType:unstructured_grid,"
    "uid:ba65665a9e68d1a8fa0352ecfcf8e496,intgrid:O96,intermediate-interpolation:nn}";


CASE("spectral (T1279) to regular lat/lon (1/1)") {
    api::MIRJob job;
    job.set("grid", std::vector<double>{1, 1});

    auto actions = plan(T1279, job);

    // truncate (linear spectral order), inverse transform to the intermediate grid, interpolate to the target grid
    EXPECT_EQUAL(actions.size(), 4);
    EXPECT_EQUAL(actions[0], "ShTruncate[truncation=179]");
    EXPECT(starts(actions[1], "ShToNamedGrid[") && ends(actions[1], ",grid=F90]"));
    EXPECT(starts(actions[2], "Gridded2RegularLL[increments=Increments[west_east=1,"));
    EXPECT(starts(actions[3], "Save["));
}


CASE("spectral (T1279), intermediate grid") {
    api::MIRJob job;

    SECTION("Gaussian target grid: inverse transform directly") {
        auto actions = plan(T1279, job.set("grid", "O320"));
        EXPECT_EQUAL(actions.size(), 3);
        EXPECT_EQUAL(actions[0], "ShTruncate[truncation=639]");
        EXPECT(starts(actions[1], "ShToNamedGrid[") && ends(actions[1], ",grid=O320]"));
        EXPECT(starts(actions[2], "Save["));
    }

    SECTION("Gaussian target grid, by gridspec: inverse transform directly") {
        auto actions = plan(T1279, job.set("grid", "{grid:O320}"));
        EXPECT_EQUAL(actions.size(), 3);
        EXPECT_EQUAL(actions[0], "ShTruncate[truncation=639]");
        EXPECT(starts(actions[1], "ShToGridSpec[") && contains(actions[1], R"(gridspec={"grid":"O320"})"));
        EXPECT(starts(actions[2], "Save["));
    }

    SECTION("rotated Gaussian target grid: intermediate grid") {
        auto actions = plan(T1279, job.set("grid", "O320").set("rotation", std::vector<double>{-40, 22}));
        EXPECT_EQUAL(actions.size(), 4);
        EXPECT_EQUAL(actions[0], "ShTruncate[truncation=639]");
        EXPECT(starts(actions[1], "ShToNamedGrid[") && ends(actions[1], ",grid=F320]"));
        EXPECT(starts(actions[2], "Gridded2RotatedNamedGrid[grid=O320,"));
        EXPECT(starts(actions[3], "Save["));
    }

    SECTION("intgrid=O640: truncation and intermediate grid from O640") {
        auto actions = plan(T1279, job.set("grid", "O320").set("intgrid", "O640"));
        EXPECT_EQUAL(actions.size(), 3);
        EXPECT(starts(actions[0], "ShToNamedGrid[") && ends(actions[0], ",grid=O640]"));
        EXPECT(starts(actions[1], "Gridded2NamedGrid[grid=O320,"));
        EXPECT(starts(actions[2], "Save["));
    }

    SECTION("intgrid=none: inverse transform directly") {
        auto actions = plan(T1279, job.set("grid", std::vector<double>{1, 1}).set("intgrid", "none"));
        EXPECT_EQUAL(actions.size(), 3);
        EXPECT_EQUAL(actions[0], "ShTruncate[truncation=179]");
        EXPECT(starts(actions[1], "ShToRegularLL["));
        EXPECT(starts(actions[2], "Save["));
    }
}


CASE("gridded (eORCA1_T), intermediate interpolation") {
    api::MIRJob job;

    SECTION("intermediate grid O96 (nn), then target grid (linear)") {
        auto actions = plan(ORCA, job.set("grid", std::vector<double>{1, 1}));
        EXPECT_EQUAL(actions.size(), 3);
        EXPECT(starts(actions[0], "Gridded2NamedGrid[grid=O96,interpolation=nn,"));
        EXPECT(starts(actions[1], "Gridded2RegularLL[") && contains(actions[1], "interpolation=linear,"));
        EXPECT(starts(actions[2], "Save["));
    }

    SECTION("target grid is the intermediate grid: one interpolation") {
        auto actions = plan(ORCA, job.set("grid", "O96"));
        EXPECT_EQUAL(actions.size(), 2);
        EXPECT(starts(actions[0], "Gridded2NamedGrid[grid=O96,interpolation=nn,"));
        EXPECT(starts(actions[1], "Save["));
    }

    SECTION("intgrid=none: no intermediate interpolation") {
        auto actions = plan(ORCA, job.set("grid", std::vector<double>{1, 1}).set("intgrid", "none"));
        EXPECT_EQUAL(actions.size(), 2);
        EXPECT(starts(actions[0], "Gridded2RegularLL[") && contains(actions[0], "interpolation=linear,"));
        EXPECT(starts(actions[1], "Save["));
    }

    SECTION("intermediate-interpolation=none: no intermediate interpolation") {
        auto actions = plan(ORCA, job.set("grid", std::vector<double>{1, 1}).set("intermediate-interpolation", "none"));
        EXPECT_EQUAL(actions.size(), 2);
        EXPECT(starts(actions[0], "Gridded2RegularLL["));
        EXPECT(starts(actions[1], "Save["));
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
