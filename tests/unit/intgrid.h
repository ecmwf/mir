// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/utils/StringTools.h"

#include "mir/action/plan/Action.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/key/grid/Grid.h"
#include "mir/key/intgrid/Intgrid.h"
#include "mir/key/style/MIRStyle.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tests::unit {


using key::intgrid::Intgrid;
using key::intgrid::IntgridFactory;


// spectral input, at truncation T (Gaussian N = T + 1, cubic spectral order)
struct SpectralField : param::SimpleParametrisation {
    explicit SpectralField(long T, bool uv = false) {
        set("spectral", true).set("truncation", T);
        if (uv) {
            set("is_wind_component_uv", 1L);
        }
    }
};


struct GriddedField : param::SimpleParametrisation {
    GriddedField() { set("gridded", true).set("grid", std::vector<double>{2, 2}); }
};


inline key::grid::Target target(const param::MIRParametrisation& user, const param::MIRParametrisation& field) {
    const param::CombinedParametrisation param(user, field);
    return key::grid::Target(param);
}


// intermediate grid, as configured by 'intgrid'
inline std::string intgrid(const param::MIRParametrisation& user, const param::MIRParametrisation& field) {
    const param::CombinedParametrisation param(user, field);

    std::string name;
    ASSERT(user.get("intgrid", name));

    std::unique_ptr<const Intgrid> grid(IntgridFactory::build(name, param));
    return grid->gridname();
}


// plan (printed actions) from post-processing a field, as prepared by the default style
struct Plan {
    Plan(const param::MIRParametrisation& user, const param::MIRParametrisation& field) {
        const param::CombinedParametrisation param(user, field);
        std::unique_ptr<key::style::MIRStyle> style(key::style::MIRStyleFactory::build(param));

        output::EmptyOutput out;
        action::ActionPlan plan(param);
        style->prepare(plan, out);

        for (const auto* action : plan) {
            ASSERT(action != nullptr);
            std::ostringstream str;
            str << *action;
            actions_.emplace_back(str.str());
        }

        Log::info() << "user: " << user << "\nplan: " << plan << std::endl;
    }

    // position of first action starting with prefix, and containing all of the (optional) substrings
    size_t find(const std::string& prefix, const std::vector<std::string>& contains = {}) const {
        auto it = std::find_if(actions_.begin(), actions_.end(), [&](const std::string& action) {
            return eckit::StringTools::startsWith(action, prefix + "[") &&
                   std::all_of(contains.begin(), contains.end(), [&](const std::string& substring) {
                       return action.find(substring) != std::string::npos;
                   });
        });
        return it == actions_.end() ? npos : static_cast<size_t>(it - actions_.begin());
    }

    bool has(const std::string& prefix, const std::vector<std::string>& contains = {}) const {
        return find(prefix, contains) != npos;
    }

    // interpolation, from gridded to gridded
    bool interpolates() const {
        return std::any_of(actions_.begin(), actions_.end(), [](const std::string& action) {
            return eckit::StringTools::startsWith(action, "Gridded2");
        });
    }

    static constexpr size_t npos = std::string::npos;

private:
    std::vector<std::string> actions_;
};


}  // namespace mir::tests::unit
