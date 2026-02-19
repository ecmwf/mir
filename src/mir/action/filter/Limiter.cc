/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/filter/Limiter.h"

#include <algorithm>
#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/grib/Config.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::action::filter {


static const ActionBuilder<Limiter> __action("filter.limiter");


Limiter::Limiter(const param::MIRParametrisation& param) : Action(param) {
    static const grib::Config config(LibMir::configFile(LibMir::config_file::LIMITER), false);
    const auto& limits = config.find(param);

    auto get = [](const param::MIRParametrisation& param, const std::string& key, double dfault) -> double {
        auto value = dfault;
        param.get(key, value);
        return value;
    };

    minimum_ = get(param, "limiter-minimum", get(limits, "minimum", -1.e30));
    maximum_ = get(param, "limiter-maximum", get(limits, "maximum", 1.e30));

    if (minimum_ > maximum_) {
        throw exception::UserError("Limiter: minimum (" + std::to_string(minimum_) +
                                   ") cannot be greater than maximum (" + std::to_string(maximum_) + ")");
    }
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
