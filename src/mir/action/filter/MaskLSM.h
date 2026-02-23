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
