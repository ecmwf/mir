// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>
#include <vector>

#include "mir/util/Formula.h"


namespace mir::util {
class Function;
}  // namespace mir::util


namespace mir::util {

class FormulaFunction : public Formula {
public:
    FormulaFunction(const param::MIRParametrisation& parametrisation, const std::string& name, Formula* arg1);

    FormulaFunction(const param::MIRParametrisation& parametrisation, const std::string& name, Formula* arg1,
                    Formula* arg2);

    FormulaFunction(const param::MIRParametrisation& parametrisation, const std::string& name,
                    std::vector<Formula*>& args);

    ~FormulaFunction() override;

protected:  // members
    const Function& function_;
    std::vector<Formula*> args_;

private:
    void print(std::ostream&) const override;
    void execute(context::Context&) const override;
    bool sameAs(const Action&) const override;
    const char* name() const override;
};


}  // namespace mir::util
