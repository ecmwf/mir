// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/action/plan/Action.h"


namespace mir::action::filter {


class Limiter final : public Action {
public:
    explicit Limiter(const param::MIRParametrisation&);

private:
    double minimum_;
    double maximum_;

    bool sameAs(const Action&) const override;
    const char* name() const override;
    void print(std::ostream&) const override;
    void execute(context::Context&) const override;
};


}  // namespace mir::action::filter
