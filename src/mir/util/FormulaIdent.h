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
