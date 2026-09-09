// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>

#include "mir/util/Formula.h"


namespace mir::util {


class FormulaString : public Formula {
public:
    FormulaString(const param::MIRParametrisation& parametrisation, const std::string& value);
    ~FormulaString() override;

private:
    void print(std::ostream&) const override;
    void execute(context::Context&) const override;
    bool sameAs(const Action&) const override;
    const char* name() const override;

private:  // members
    std::string value_;
};


}  // namespace mir::util
