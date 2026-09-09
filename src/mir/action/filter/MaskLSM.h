// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>
#include <vector>

#include "mir/action/plan/Action.h"


namespace mir::action::filter {


class MaskLSM : public Action {
public:
    struct Value {
        static void list(std::ostream&);
        static bool get(const param::MIRParametrisation&, const std::string&);
    };

protected:
    MaskLSM(const param::MIRParametrisation&, const std::string&);
    int value() const { return value_ ? 1 : 0; }

private:
    virtual const std::vector<bool>& mask(const repres::Representation&) const = 0;

    void print(std::ostream&) const final;
    void execute(context::Context&) const final;

    const bool value_;
};


}  // namespace mir::action::filter
