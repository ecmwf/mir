// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <algorithm>
#include <cstdlib>
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


/// Unstructured gridspec of the points of a file (in the test data), "latitude longitude" per line
inline std::string unstructured(const eckit::PathName& path) {
    std::ifstream in(path.asString());
    ASSERT(in);

    std::string lat;
    std::string lon;
    std::string lats;
    std::string lons;
    while (in >> lat >> lon) {
        lats += (lats.empty() ? "" : ",") + lat;
        lons += (lons.empty() ? "" : ",") + lon;
    }

    return "{type:unstructured_ll,latitudes:[" + lats + "],longitudes:[" + lons + "]}";
}


/// Key value as a string (as grib_get -p)
inline std::string get(const handle_t& h, const std::string& key) {
    char value[1024];
    size_t length = sizeof(value);
    GRIB_CALL(codes_get_string(h.get(), key.c_str(), value, &length));
    return value;
}


/// Keys of different values between two messages (arrays by value, other keys as strings), for a readable failure
inline std::string different_keys(codes_handle* a, codes_handle* b) {
    // but the reference value and binary scale factor where decoding doesn't read them (GRIB1 IEEE packing, or without
    // values)
    long edition      = 0;
    long values       = 0;
    char packing[64]  = {0};
    size_t packingLen = sizeof(packing);
    codes_get_long(a, "edition", &edition);
    codes_get_long(a, "numberOfValues", &values);
    codes_get_string(a, "packingType", packing, &packingLen);

    const auto unused = values == 0 || (edition == 1 && std::string(packing) == "grid_ieee");

    std::string diff;
    auto append = [&diff](const std::string& difference) { diff += (diff.empty() ? "" : ", ") + difference; };

    std::unique_ptr<codes_keys_iterator, decltype(&codes_keys_iterator_delete)> it(
        codes_keys_iterator_new(a, CODES_KEYS_ITERATOR_SKIP_COMPUTED | CODES_KEYS_ITERATOR_SKIP_DUPLICATES, nullptr),
        &codes_keys_iterator_delete);

    while (codes_keys_iterator_next(it.get()) != 0) {
        const std::string key = codes_keys_iterator_get_name(it.get());
        if (unused && (key == "referenceValue" || key == "binaryScaleFactor")) {
            continue;
        }

        if (size_t na = 0, nb = 0; codes_get_size(a, key.c_str(), &na) == CODES_SUCCESS && na > 1) {
            std::vector<double> xa(na);
            std::vector<double> xb;
            if (codes_get_size(b, key.c_str(), &nb) == CODES_SUCCESS && nb == na) {
                xb.resize(nb);
                codes_get_double_array(a, key.c_str(), xa.data(), &na);
                codes_get_double_array(b, key.c_str(), xb.data(), &nb);
            }

            if (xa != xb) {
                append(key + "[" + std::to_string(na) + "/" + std::to_string(nb) + "]");
            }
            continue;
        }

        char va[1024];
        char vb[1024];
        size_t la = sizeof(va);
        size_t lb = sizeof(vb);
        if (codes_get_string(a, key.c_str(), va, &la) != CODES_SUCCESS) {
            continue;
        }

        if (codes_get_string(b, key.c_str(), vb, &lb) != CODES_SUCCESS) {
            append(key + "=" + va + "/?");
        }
        else if (std::string(va) != std::string(vb)) {
            append(key + "=" + va + "/" + vb);
        }
    }

    return diff;
}


/// Post-processing an artificial input (as the mir tool with --input) to GRIB, checking the action and grid plans
/// (as --dump-action-plan-file and --dump-grid-plan-file, a grid plan starting with '#' commented), the output message,
/// encoded identically by GRIB keys and by gridSpec (MIR_GRIDSPEC_GRIB_OUTPUT)
inline handle_t run(const std::string& input, const options_t& options, const std::string& actionPlan,
                    const std::string& gridPlan) {
    // plan files in the temporary directory (not the working directory, the source tree), removed in any case
    struct Temporary {
        explicit Temporary(const std::string& name) :
            path(eckit::PathName::unique((std::filesystem::temp_directory_path() / name).string())) {}
        ~Temporary() { path.unlink(false); }
        Temporary(const Temporary&)            = delete;
        Temporary& operator=(const Temporary&) = delete;
        const eckit::PathName path;
    };

    auto encode = [&](bool gridspec) {
        ::setenv("MIR_GRIDSPEC_GRIB_OUTPUT", gridspec ? "1" : "0", 1);

        param::SimpleParametrisation param;
        param.set("input", input);
        std::unique_ptr<input::MIRInput> in(input::MIRInputFactory::build("/dev/null", param));

        const Temporary actionPlan_("mir-action-plan");
        const Temporary gridPlan_("mir-grid-plan");

        api::MIRJob job;

        // computing anew (weights cached by a former version could hide a difference)
        job.set("caching", false);

        for (const auto& kv : options) {
            set(job, kv.first, kv.second);
        }

        job.set("dump-action-plan-file", actionPlan_.path.asString());
        job.set("dump-grid-plan-file", gridPlan_.path.asString());

        static std::vector<char> buffer(64 * 1024 * 1024);
        output::GribMemoryOutput out(buffer.data(), buffer.size());
        job.execute(*in, out);

        EXPECT_EQUAL(contents(actionPlan_.path), actionPlan);
        if (gridPlan.rfind('#', 0) != 0) {
            EXPECT_EQUAL(contents(gridPlan_.path), gridPlan);
        }

        return std::string(buffer.data(), out.length());
    };

    const auto legacy   = encode(false);
    const auto gridspec = encode(true);
    ::unsetenv("MIR_GRIDSPEC_GRIB_OUTPUT");

    handle_t h(codes_handle_new_from_message_copy(nullptr, legacy.data(), legacy.size()), &codes_handle_delete);
    handle_t g(codes_handle_new_from_message_copy(nullptr, gridspec.data(), gridspec.size()), &codes_handle_delete);
    ASSERT(h && g);

    // the same message, or the same grid (random values do not repeat)
    if (legacy != gridspec) {
        auto random =
            std::any_of(options.begin(), options.end(), [](const auto& kv) { return kv.first == "add-random"; });
        const auto difference = random ? get(h, "md5GridSection") : different_keys(h.get(), g.get());
        const auto expected   = random ? get(g, "md5GridSection") : std::string();
        EXPECT_EQUAL(difference, expected);
    }

    return h;
}


}  // namespace mir::tests::regressions
