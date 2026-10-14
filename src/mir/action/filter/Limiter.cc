// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/filter/Limiter.h"

#include <algorithm>
#include <ostream>

#include "eckit/log/JSON.h"

#include "mir/action/context/Context.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/grib/Config.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::action::filter {


static const ActionBuilder<Limiter> __action("filter.limiter");


Limiter::Limiter(const param::MIRParametrisation& param) : Action(param), minimum_(-1.e30), maximum_(1.e30) {
    // as given, or as configured (by parameter, only if needed), or unlimited
    if (!param.has("limiter-minimum") || !param.has("limiter-maximum")) {
        static const grib::Config config(LibMir::configFile(LibMir::config_file::LIMITER), false);
        const auto& limits = config.find(param);

        limits.get("minimum", minimum_);
        limits.get("maximum", maximum_);
    }

    param.get("limiter-minimum", minimum_);
    param.get("limiter-maximum", maximum_);

    if (minimum_ > maximum_) {
        throw exception::UserError("Limiter: minimum (" + std::to_string(minimum_) +
                                   ") cannot be greater than maximum (" + std::to_string(maximum_) + ")");
    }
}


void Limiter::json(eckit::JSON& j) const {
    j << "type" << "filter.limiter";
    j << "limiter-minimum" << minimum_;
    j << "limiter-maximum" << maximum_;
}


bool Limiter::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Limiter*>(&other);
    return o != nullptr && minimum_ == o->minimum_ && maximum_ == o->maximum_;
}


const char* Limiter::name() const {
    return "Limiter";
}


void Limiter::print(std::ostream& out) const {
    out << name() << "[minimum=" << minimum_ << ",maximum=" << maximum_ << "]";
}


void Limiter::execute(context::Context& ctx) const {
    auto& field = ctx.field();
    auto miss   = field.missingValue();

    for (size_t i = 0; i < field.dimensions(); ++i) {
        auto& values = field.direct(i);
        std::for_each(values.begin(), values.end(), [=](auto& value) {
            value = value == miss ? miss : std::max(minimum_, std::min(maximum_, value));
        });
    }
}


}  // namespace mir::action::filter
