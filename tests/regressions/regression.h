// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "eccodes.h"

#include "eckit/filesystem/PathName.h"
#include "eckit/testing/Test.h"
#include "eckit/utils/StringTools.h"
#include "eckit/utils/Tokenizer.h"
#include "eckit/utils/Translator.h"

#include "mir/api/MIRJob.h"
#include "mir/input/MIRInput.h"
#include "mir/output/GribMemoryOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::tests::regressions {


using handle_t  = std::unique_ptr<codes_handle, decltype(&codes_handle_delete)>;
using options_t = std::vector<std::pair<std::string, std::string>>;


inline bool is_number(const std::string& s) {
    char* end = nullptr;
    std::strtod(s.c_str(), &end);
    return !s.empty() && end == s.c_str() + s.size();
}


/// Set an option as the mir tool does: flag (empty value), number, list of numbers (a/b/...), or string
inline void set(api::MIRJob& job, const std::string& key, const std::string& value) {
    if (key == "plan" || key == "plan-script") {
        job.set("style", "custom");  // as the mir tool
    }

    if (value.empty()) {
        job.set(key, true);
        return;
    }

    std::vector<std::string> v;
    eckit::Tokenizer("/")(value, v);

    bool numbers = !v.empty();

    for (const auto& x : v) {
        numbers = numbers && is_number(x);
    }

    if (!numbers) {
        job.set(key, value);
    }
    else if (v.size() > 1 || value.find('/') != std::string::npos) {
        std::vector<double> d;

        for (const auto& x : v) {
            d.push_back(eckit::Translator<std::string, double>()(x));
        }

        job.set(key, d);
    }
    else if (value.find_first_of(".eE") == std::string::npos) {
        job.set(key, eckit::Translator<std::string, long>()(value));
    }
    else {
        job.set(key, eckit::Translator<std::string, double>()(value));
    }
}


inline std::string contents(const eckit::PathName& path) {
    std::ifstream in(path.asString());
    std::string line;
    std::getline(in, line);
    return line;
}


/// Post-processing an artificial input (as the mir tool with --input) to GRIB, checking the action and grid plans
/// (as --dump-action-plan-file and --dump-grid-plan-file), the output message
inline handle_t run(const std::string& input, const options_t& options, const std::string& actionPlan,
                    const std::string& gridPlan) {
    param::SimpleParametrisation param;
    param.set("input", input);
    std::unique_ptr<input::MIRInput> in(input::MIRInputFactory::build("/dev/null", param));

    // plan files in the temporary directory (not the working directory, the source tree), removed in any case
    struct Temporary {
        explicit Temporary(const std::string& name) :
            path(eckit::PathName::unique((std::filesystem::temp_directory_path() / name).string())) {}
        ~Temporary() { path.unlink(false); }
        Temporary(const Temporary&)            = delete;
        Temporary& operator=(const Temporary&) = delete;
        const eckit::PathName path;
    };

    const Temporary actionPlan_("mir-action-plan");
    const Temporary gridPlan_("mir-grid-plan");
    const auto& actionPlanFile = actionPlan_.path;
    const auto& gridPlanFile   = gridPlan_.path;

    api::MIRJob job;

    for (const auto& kv : options) {
        set(job, kv.first, kv.second);
    }

    job.set("dump-action-plan-file", actionPlanFile.asString());
    job.set("dump-grid-plan-file", gridPlanFile.asString());

    static std::vector<char> buffer(64 * 1024 * 1024);
    output::GribMemoryOutput out(buffer.data(), buffer.size());
    job.execute(*in, out);

    EXPECT_EQUAL(contents(actionPlanFile), actionPlan);
    EXPECT_EQUAL(contents(gridPlanFile), gridPlan);

    handle_t h(codes_handle_new_from_message_copy(nullptr, buffer.data(), out.length()), &codes_handle_delete);
    ASSERT(h);
    return h;
}


/// Key value as a string (as grib_get -p)
inline std::string get(const handle_t& h, const std::string& key) {
    char value[1024];
    size_t length = sizeof(value);
    GRIB_CALL(codes_get_string(h.get(), key.c_str(), value, &length));
    return value;
}


}  // namespace mir::tests::regressions
