// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/filesystem/TmpFile.h"
#include "eckit/testing/Test.h"
#include "eckit/utils/StringTools.h"

#include "mir/action/plan/Action.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/action/plan/Job.h"
#include "mir/api/MIRJob.h"
#include "mir/input/MIRInput.h"
#include "mir/key/style/CustomParametrisation.h"
#include "mir/key/style/MIRStyle.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"


namespace mir::action::tests {


/// Plan prepared (not executed) for a job on an (artificial) input, by style (see key::style::MIRStyle)
struct prepared_t {
    prepared_t(const std::string& artificialInput, const api::MIRJob& request) :
        input(build(artificialInput)), job(request, *input, output, false) {
        Log::info() << job.plan() << std::endl;
    }

    static input::MIRInput* build(const std::string& artificialInput) {
        param::SimpleParametrisation field;
        field.set("input", artificialInput);
        return input::MIRInputFactory::build("constant", field);
    }

    std::unique_ptr<input::MIRInput> input;
    output::EmptyOutput output;
    const action::Job job;
};


// plan (printed actions), prepared but not executed
std::vector<std::string> plan(const std::string& artificialInput, const api::MIRJob& job) {
    const prepared_t prepared(artificialInput, job);

    std::vector<std::string> actions;
    for (const auto* action : prepared.job.plan()) {
        std::ostringstream str;
        str << *action;
        actions.emplace_back(str.str());
    }
    return actions;
}


// plan (action types), prepared but not executed
std::vector<std::string> types(const std::string& artificialInput, const api::MIRJob& job) {
    const prepared_t prepared(artificialInput, job);

    std::vector<std::string> types;
    for (const auto* action : prepared.job.plan()) {
        types.emplace_back(action->type());
    }
    return types;
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
const std::string T63   = "{artificialInput:constant,constant:0.,spectral:true,truncation:63,gridType:sh}";
const std::string T63_UV =
    "{artificialInput:constant,constant:0.,spectral:true,truncation:63,gridType:sh,is_wind_component_uv:1}";
const std::string O32 = "{artificialInput:gridspec,gridSpec:{grid:O32}}";
const std::string LL1 = "{artificialInput:gridspec,gridSpec:{grid:[1,1]}}";

// eORCA1_T, with the intermediate grid and interpolation from the grid catalog and grid-type.yaml
const std::string ORCA =
    "{artificialInput:constant,constant:0.,gridded:true,gridType:unstructured_grid,"
    "uid:ba65665a9e68d1a8fa0352ecfcf8e496,intgrid:O96,intermediate-interpolation:nn}";

CASE("ecmwf style, spectral (T1279) to regular lat/lon (1/1)") {
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


CASE("ecmwf style, spectral (T1279), intermediate grid") {
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

    SECTION("regional Gaussian target grid, by gridspec: inverse transform to the global grid, cropped") {
        auto actions = plan(T1279, job.set("grid", "{grid:O320,area:[60,-10,30,40]}"));
        EXPECT_EQUAL(actions.size(), 3);
        EXPECT_EQUAL(actions[0], "ShTruncate[truncation=639]");
        EXPECT(starts(actions[1], "ShToGridSpec[type=local,cropping=BoundingBox[") &&
               contains(actions[1], R"(gridspec={"grid":"O320"})"));
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


CASE("ecmwf style, gridded (eORCA1_T), intermediate interpolation") {
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


CASE("ecmwf style, gridded input, target grid") {
    using V = std::vector<std::string>;
    api::MIRJob job;

    SECTION("regular lat/lon") {
        EXPECT(types(O32, job.set("grid", std::vector<double>{10, 10})) == V({"interpolate.grid2regular-ll", "save"}));
    }

    SECTION("rotated regular lat/lon") {
        job.set("grid", std::vector<double>{10, 10}).set("rotation", std::vector<double>{-40, 22});
        EXPECT(types(O32, job) == V({"interpolate.grid2rotated-regular-ll", "save"}));
    }

    SECTION("named grid") {
        EXPECT(types(O32, job.set("grid", "O16")) == V({"interpolate.grid2namedgrid", "save"}));
    }

    SECTION("Gaussian grids, by number") {
        const std::vector<std::string> keys{"reduced", "regular", "octahedral"};
        for (const std::string& key : keys) {
            api::MIRJob gaussian;
            gaussian.set(key, 16L);
            EXPECT(types(O32, gaussian) == V({"interpolate.grid2" + key + "-gg", "save"}));

            gaussian.set("rotation", std::vector<double>{-40, 22});
            EXPECT(types(O32, gaussian) == V({"interpolate.grid2rotated-" + key + "-gg", "save"}));
        }
    }

    SECTION("Gaussian grid, pl given") {
        job.set("pl", std::vector<long>{20, 24, 28, 32, 32, 28, 24, 20});
        EXPECT(types(O32, job) == V({"interpolate.grid2reduced-gg-pl-given", "save"}));

        job.set("rotation", std::vector<double>{-40, 22});
        EXPECT(types(O32, job) == V({"interpolate.grid2rotated-reduced-gg-pl-given", "save"}));
    }

    SECTION("griddef") {
        EXPECT(types(O32, job.set("griddef", "tour.pts")) == V({"interpolate.grid2griddef", "save"}));
    }

    SECTION("points") {
        job.set("latitudes", std::vector<double>{10, 20}).set("longitudes", std::vector<double>{0, 10});
        EXPECT(types(O32, job) == V({"interpolate.grid2points", "save"}));
    }

    SECTION("no interpolation") {
        job.set("grid", std::vector<double>{10, 10}).set("interpolation", "none");
        EXPECT(types(O32, job) == V({"save"}));
    }
}


CASE("ecmwf style, gridded input, target grid errors") {
    api::MIRJob job;

    SECTION("griddef, rotated") {
        job.set("griddef", "tour.pts").set("rotation", std::vector<double>{-40, 22});
        EXPECT_THROWS(types(O32, job));
    }

    SECTION("points, without longitudes") {
        EXPECT_THROWS(types(O32, job.set("latitudes", std::vector<double>{10, 20})));
    }

    SECTION("points, rotated") {
        job.set("latitudes", std::vector<double>{10, 20}).set("longitudes", std::vector<double>{0, 10});
        EXPECT_THROWS(types(O32, job.set("rotation", std::vector<double>{-40, 22})));
    }

    SECTION("rotated regular lat/lon, masked area") {
        job.set("grid", std::vector<double>{10, 10}).set("rotation", std::vector<double>{-40, 22});
        job.set("area", std::vector<double>{60, -10, 30, 40}).set("area-mode", "mask");
        EXPECT_THROWS(types(O32, job));
    }

    SECTION("more than one target grid") {
        EXPECT_THROWS(types(O32, job.set("grid", "O16").set("reduced", 16L)));
    }

    SECTION("vorticity/divergence") {
        EXPECT_THROWS(types(O32, job.set("grid", "O16").set("vod2uv", true)));
    }
}


CASE("ecmwf style, regular lat/lon input") {
    using V = std::vector<std::string>;
    api::MIRJob job;

    SECTION("the same grid, aligned area: cropping only") {
        job.set("grid", std::vector<double>{1, 1}).set("area", std::vector<double>{10, 0, -10, 20});
        EXPECT(types(LL1, job) == V({"crop.area", "save"}));
    }

    SECTION("the same grid, shifted area: interpolation") {
        job.set("grid", std::vector<double>{1, 1}).set("area", std::vector<double>{10.5, 0.5, -10.5, 20.5});
        EXPECT(types(LL1, job) == V({"interpolate.grid2regular-ll", "crop.area", "save"}));
    }

    SECTION("area: cropping only") {
        EXPECT(types(LL1, job.set("area", std::vector<double>{10, 0, -10, 20})) == V({"crop.area", "save"}));
    }

    SECTION("area, by masking") {
        job.set("area", std::vector<double>{10, 0, -10, 20}).set("area-mode", "mask");
        EXPECT(types(LL1, job) == V({"mask.area", "save"}));
    }

    SECTION("rotation: interpolation to the rotated grid") {
        job.set("rotation", std::vector<double>{-40, 22});
        EXPECT(types(LL1, job) == V({"interpolate.grid2rotated-regular-ll", "save"}));
    }
}


CASE("ecmwf style, gridded input, winds") {
    using V = std::vector<std::string>;
    api::MIRJob job;
    job.set("grid", std::vector<double>{10, 10}).set("uv2uv", true);

    SECTION("rotated: wind directions adjusted") {
        job.set("rotation", std::vector<double>{-40, 22});
        EXPECT(types(O32, job) == V({"interpolate.grid2rotated-regular-ll", "filter.adjust-winds-directions", "save"}));
    }

    SECTION("u only") {
        auto actions = plan(O32, job.set("u-only", true));
        EXPECT_EQUAL(actions.size(), 3);
        EXPECT_EQUAL(actions[1], "SelectField[0]");
    }

    SECTION("v only") {
        auto actions = plan(O32, job.set("v-only", true));
        EXPECT_EQUAL(actions.size(), 3);
        EXPECT_EQUAL(actions[1], "SelectField[1]");
    }
}


CASE("ecmwf style, spectral input") {
    using V = std::vector<std::string>;
    api::MIRJob job;
    job.set("truncation", 21L);

    SECTION("truncation") {
        auto actions = plan(T63, job);
        EXPECT_EQUAL(actions.size(), 2);
        EXPECT_EQUAL(actions[0], "ShTruncate[truncation=21]");
        EXPECT(starts(actions[1], "Save["));
    }

    SECTION("spectral filters") {
        job.set("cesaro", true).set("bandpass", std::vector<long>{0, 10, 0, 20});
        EXPECT(types(T63, job) ==
               V({"filter.sh-truncate", "filter.sh-cesaro-summation-filter", "filter.sh-bandpass", "save"}));
    }

    SECTION("formula") {
        job.set("formula.spectral", "f * 2").set("formula.spectral.metadata", "paramId=1");
        EXPECT(types(T63, job) == V({"filter.sh-truncate", "calc.formula", "save"}));
    }

    SECTION("vorticity/divergence to U/V") {
        EXPECT(types(T63, job.set("vod2uv", true)) == V({"filter.sh-truncate", "transform.sh-vod-to-UV", "save"}));
    }
}


CASE("ecmwf style, spectral input, to grid") {
    using V = std::vector<std::string>;
    api::MIRJob job;
    job.set("truncation", 21L).set("grid", std::vector<double>{10, 10});

    SECTION("no intermediate grid") {
        EXPECT(types(T63, job.set("intgrid", "none")) ==
               V({"filter.sh-truncate", "transform.sh-scalar-to-regular-ll", "save"}));
    }

    SECTION("U/V, no intermediate grid") {
        job.set("intgrid", "none").set("uv2uv", true);
        EXPECT(types(T63, job) == V({"filter.sh-truncate", "transform.sh-scalar-to-regular-ll",
                                     "filter.adjust-winds-scale-cos-latitude", "save"}));
    }

    SECTION("U/V, intermediate grid: scaling before interpolation") {
        EXPECT(types(T63, job.set("uv2uv", true)) ==
               V({"filter.sh-truncate", "transform.sh-scalar-to-namedgrid", "filter.adjust-winds-scale-cos-latitude",
                  "interpolate.grid2regular-ll", "save"}));
    }

    SECTION("U/V, intermediate grid, compatible: scaling after interpolation") {
        job.set("uv2uv", true).set("sh2grid-wind", "compatible");
        EXPECT(types(T63, job) == V({"filter.sh-truncate", "transform.sh-scalar-to-namedgrid",
                                     "interpolate.grid2regular-ll", "filter.adjust-winds-scale-cos-latitude", "save"}));
    }

    SECTION("U/V, rotated: wind directions adjusted") {
        job.set("uv2uv", true).set("rotation", std::vector<double>{-40, 22});
        EXPECT(types(T63, job) ==
               V({"filter.sh-truncate", "transform.sh-scalar-to-namedgrid", "filter.adjust-winds-scale-cos-latitude",
                  "interpolate.grid2rotated-regular-ll", "filter.adjust-winds-directions", "save"}));
    }

    SECTION("vorticity/divergence to U/V, V only") {
        job.set("vod2uv", true).set("v-only", true);
        EXPECT(types(T63, job) == V({"filter.sh-truncate", "transform.sh-vod-to-uv-namedgrid",
                                     "interpolate.grid2regular-ll", "select.field", "save"}));
    }

    SECTION("formula, on the gridded field") {
        EXPECT(types(T63, job.set("formula.gridded", "f + 1")) ==
               V({"filter.sh-truncate", "transform.sh-scalar-to-namedgrid", "interpolate.grid2regular-ll",
                  "calc.formula", "save"}));
    }

    SECTION("intermediate grid, compatible") {
        const auto reference = types(T63, job);
        EXPECT_EQUAL(reference.size(), 4);
        EXPECT_EQUAL(reference[1], "transform.sh-scalar-to-namedgrid");

        EXPECT(types(T63, job.set("sh2grid-intgrid", "compatible")) == reference);

        // unset options take the first choice
        EXPECT(types(T63, job.set("sh2grid-intgrid", "").set("sh2grid-wind", "")) == reference);
    }

    SECTION("U/V input") {
        job.set("intgrid", "none");
        EXPECT(types(T63_UV, job) == V({"filter.sh-truncate", "transform.sh-scalar-to-regular-ll",
                                        "filter.adjust-winds-scale-cos-latitude", "save"}));
    }

    SECTION("errors") {
        struct test_t {
            std::string input;
            std::string key;
            std::string value;
        };

        const std::vector<test_t> tests{
            {T63, "uv2uv", "1"},  // with vod2uv
            {T63_UV, "", ""},     // with vod2uv
            {T63, "sh2grid-wind", "unknown"},
            {T63, "sh2grid-intgrid", "unknown"},
        };
        for (const auto& test : tests) {
            api::MIRJob error;
            error.set("grid", std::vector<double>{10, 10});

            if (test.key.empty() || test.key == "uv2uv") {
                error.set("vod2uv", true);
            }

            if (!test.key.empty()) {
                error.set(test.key, test.value);
            }

            EXPECT_THROWS(types(test.input, error));
        }
    }
}


CASE("ecmwf style, options") {
    api::MIRJob job;
    job.set("grid", "O16");

    job.set("prologue", "noop").set("epilogue", "noop");
    job.set("checkerboard", true).set("pattern", true).set("reset-missing-values", true);
    job.set("formula.prologue", "f + 1").set("formula.epilogue", "f * 2");
    job.set("metadata", "paramId=1");
    job.set("add-random", "uniform-int-distribution{a:1,b:3}");
    job.set("limiter", true).set("limiter-minimum", 0.).set("limiter-maximum", 1.);
    job.set("pre-globalise", true).set("globalise", true);
    job.set("mask-input-lsm-value", 1L).set("mask-output-lsm-value", 0L);
    job.set("nabla", "scalar-gradient/scalar-laplacian");
    job.set("area", std::vector<double>{60, -10, 30, 40});
    job.set("bitmap", "ICE.1").set("frame", 2L).set("unstructured", true);

    const std::vector<std::string> prologue{"noop",
                                            "misc.checkerboard",
                                            "misc.pattern",
                                            "misc.reset-missing-values",
                                            "filter.statistics",
                                            "calc.formula",
                                            "filter.globalise",
                                            "filter.mask-input-lsm",
                                            "interpolate.grid2namedgrid",
                                            "filter.scalar-gradient",
                                            "filter.scalar-laplacian",
                                            "filter.globalise"};

    const std::vector<std::string> epilogue{
        "filter.bitmap", "filter.mask-output-lsm", "filter.frame",      "filter.unstructured", "calc.formula",
        "set.metadata",  "filter.statistics",      "filter.add-random", "filter.limiter",      "noop",
        "save"};

    auto expected = [&prologue, &epilogue](const std::string& area) {
        auto all = prologue;
        all.push_back(area);
        all.insert(all.end(), epilogue.begin(), epilogue.end());
        return all;
    };

    SECTION("cropping") {
        EXPECT(types(O32, job.set("statistics", "min-max")) == expected("crop.area"));
    }

    SECTION("masking") {
        job.set("statistics", "min-max").set("area-mode", "mask");
        EXPECT(types(O32, job) == expected("mask.area"));
    }

    SECTION("input/output statistics") {
        auto input = types(O32, job.set("input-statistics", "min-max"));
        EXPECT_EQUAL(input.size(), expected("crop.area").size() - 1);
        EXPECT_EQUAL(input[4], "filter.statistics");

        job.clear("input-statistics");
        auto output = types(O32, job.set("output-statistics", "min-max"));
        EXPECT_EQUAL(output.size(), expected("crop.area").size() - 1);
        EXPECT_EQUAL(output[output.size() - 5], "filter.statistics");
    }
}


CASE("custom style") {
    const std::string PLAN =
        "frame(2);crop.area(area=[60,-10,30,40]);filter.statistics(statistics=[min-max,scalar],precision=3)";

    auto check = [](const action::ActionPlan& plan) {
        EXPECT_EQUAL(plan.size(), 4);
        EXPECT_EQUAL(plan.action(0).json_str(), R"({"type":"filter.frame","frame":2})");
        EXPECT_EQUAL(plan.action(1).json_str(), R"({"type":"crop.area","area":[60,-10,30,40]})");
        EXPECT_EQUAL(plan.action(2).json_str(),
                     R"({"type":"filter.statistics","statistics":"min-max/scalar","precision":3})");
        EXPECT_EQUAL(plan.action(3).type(), "save");
    };

    api::MIRJob job;
    job.set("style", "custom");

    SECTION("plan, actions by (partial) name") {
        // the second time, partial names are known
        check(prepared_t(O32, job.set("plan", PLAN)).job.plan());
        check(prepared_t(O32, job.set("plan", PLAN)).job.plan());
    }

    SECTION("plan script") {
        eckit::TmpFile script;
        {
            std::ofstream out(script.asString());
            out << PLAN;
        }

        check(prepared_t(O32, job.set("plan-script", script.asString())).job.plan());
    }

    SECTION("errors") {
        EXPECT_THROWS(types(O32, job));  // no plan
        EXPECT_THROWS(types(O32, job.set("plan-script", "does-not-exist.plan")));
        EXPECT_THROWS(types(O32, job.set("plan", "area(area=[60,-10,30,40])")));  // ambiguous action
        EXPECT_THROWS(types(O32, job.set("plan", "unknown(1)")));
    }
}


CASE("custom style, parametrisation") {
    param::SimpleParametrisation base;
    base.set("base", 1L);

    const std::map<std::string, std::vector<std::string>> params{
        {"text", {"a", "b"}}, {"number", {"2"}}, {"real", {"2.5"}}, {"flag", {"1"}}, {"list", {"1", "2", "3"}}};

    const key::style::CustomParametrisation custom("test", params, base);
    const param::MIRParametrisation& parametrisation = custom;

    EXPECT(&parametrisation.userParametrisation() == &parametrisation);
    EXPECT(&parametrisation.fieldParametrisation() == &parametrisation);

    std::ostringstream print;
    print << parametrisation;

    EXPECT(starts(print.str(), "CustomParametrisation[name=test,params=["));
    EXPECT(contains(print.str(), "list=[1,2,3]"));

    // values as given, by type
    std::string text;
    EXPECT(parametrisation.get("text", text) && text == "a/b");

    int i  = 0;
    long l = 0;
    EXPECT(parametrisation.get("number", i) && i == 2);
    EXPECT(parametrisation.get("number", l) && l == 2);

    float f  = 0;
    double d = 0;
    EXPECT(parametrisation.get("real", f) && f == 2.5F);
    EXPECT(parametrisation.get("real", d) && d == 2.5);

    bool b = false;
    EXPECT(parametrisation.get("flag", b) && b);

    std::vector<int> vi;
    std::vector<long> vl;
    std::vector<float> vf;
    std::vector<double> vd;
    EXPECT(parametrisation.get("list", vi) && vi == std::vector<int>({1, 2, 3}));
    EXPECT(parametrisation.get("list", vl) && vl == std::vector<long>({1, 2, 3}));
    EXPECT(parametrisation.get("list", vf) && vf == std::vector<float>({1, 2, 3}));
    EXPECT(parametrisation.get("list", vd) && vd == std::vector<double>({1, 2, 3}));

    // one value expected
    EXPECT_THROWS(parametrisation.get("text", l));

    // otherwise, from the base parametrisation
    EXPECT(parametrisation.has("number"));
    EXPECT(parametrisation.has("base"));
    EXPECT(!parametrisation.has("none"));
    EXPECT(parametrisation.get("base", l) && l == 1);
    EXPECT(!parametrisation.get("none", text));
}


CASE("styles, factory") {
    param::SimpleParametrisation param;
    EXPECT_THROWS(std::unique_ptr<key::style::MIRStyle>(key::style::MIRStyleFactory::build(param)));  // no style

    param.set("style", "unknown");
    EXPECT_THROWS(std::unique_ptr<key::style::MIRStyle>(key::style::MIRStyleFactory::build(param)));

    std::ostringstream list;
    key::style::MIRStyleFactory::list(list);

    EXPECT(contains(list.str(), "custom"));
    EXPECT(contains(list.str(), "ecmwf"));

    struct test_t {
        std::string style;
        std::string print;
    };

    const std::vector<test_t> tests{{"ecmwf", "ECMWFStyle[]"}, {"custom", "CustomStyle[]"}};
    for (const auto& test : tests) {
        param.set("style", test.style);
        std::unique_ptr<key::style::MIRStyle> style(key::style::MIRStyleFactory::build(param));

        std::ostringstream print;
        print << *style;
        EXPECT_EQUAL(print.str(), test.print);
    }

    // custom style, no plan
    param.set("style", "custom");
    std::unique_ptr<key::style::MIRStyle> style(key::style::MIRStyleFactory::build(param));

    param::RuntimeParametrisation runtime(param::DefaultParametrisation::instance());
    action::ActionPlan plan(runtime);
    output::EmptyOutput output;
    EXPECT_THROWS(style->prepare(plan, output));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
