// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

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
#include "mir/key/style/MIRStyle.h"
#include "mir/output/EmptyOutput.h"
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
inline std::vector<std::string> plan(const std::string& artificialInput, const api::MIRJob& job) {
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
inline std::vector<std::string> types(const std::string& artificialInput, const api::MIRJob& job) {
    const prepared_t prepared(artificialInput, job);

    std::vector<std::string> types;
    for (const auto* action : prepared.job.plan()) {
        types.emplace_back(action->type());
    }
    return types;
}


inline bool starts(const std::string& action, const std::string& prefix) {
    return eckit::StringTools::startsWith(action, prefix);
}


inline bool ends(const std::string& action, const std::string& suffix) {
    return eckit::StringTools::endsWith(action, suffix);
}


inline bool contains(const std::string& action, const std::string& substring) {
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


}  // namespace mir::action::tests
