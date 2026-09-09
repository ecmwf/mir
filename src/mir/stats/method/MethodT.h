// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <ostream>
#include <vector>

#include "mir/stats/Method.h"
#include "mir/stats/detail/Counter.h"


namespace mir::data {
class MIRField;
}  // namespace mir::data


namespace mir::stats::method {


/// Generic statistics on a MIRField
template <typename STATS>
class MethodT : public Method, public detail::Counter, protected std::vector<STATS> {
private:
    // -- Exceptions

    using vector_t = std::vector<STATS>;

public:
    // -- Exceptions
    // None

    // -- Constructors

    MethodT(const param::MIRParametrisation& parametrisation) : Method(parametrisation), Counter(parametrisation) {}

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void resize(size_t) override;
    void execute(const data::MIRField&) override;
    void mean(data::MIRField&) const override;
    void variance(data::MIRField&) const override;
    void stddev(data::MIRField&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods

    using vector_t::empty;
    using vector_t::size;

    // -- Overridden methods

    void print(std::ostream& out) const override {
        out << "Method[";
        Counter::print(out);
        out << "]";
    }

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::stats::method
