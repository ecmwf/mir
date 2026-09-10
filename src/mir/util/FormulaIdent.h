// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>

#include "mir/util/Formula.h"


namespace mir::util {


class FormulaIdent : public Formula {
public:
    FormulaIdent(const param::MIRParametrisation& parametrisation, const std::string& name);
    ~FormulaIdent() override;

private:
    void print(std::ostream&) const override;
    void execute(context::Context&) const override;
    bool sameAs(const Action&) const override;
    const char* name() const override;

private:  // members
    std::string name_;
};


}  // namespace mir::util
