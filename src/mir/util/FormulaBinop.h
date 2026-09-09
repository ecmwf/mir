// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>

#include "mir/util/FormulaFunction.h"


namespace mir::util {


class FormulaBinop : public FormulaFunction {
public:
    FormulaBinop(const param::MIRParametrisation& parametrisation, const std::string& name, Formula* arg1,
                 Formula* arg2);

    ~FormulaBinop() override;

private:
    void print(std::ostream&) const override;
};


}  // namespace mir::util
