// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/util/Formula.h"


namespace mir::util {


class FormulaNumber : public Formula {
public:
    FormulaNumber(const param::MIRParametrisation& parametrisation, double value);
    ~FormulaNumber() override;

private:
    void print(std::ostream&) const override;
    void execute(context::Context&) const override;
    bool sameAs(const Action&) const override;
    const char* name() const override;

private:  // members
    double value_;
};


}  // namespace mir::util
