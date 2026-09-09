// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <ostream>

#include "mir/data/MIRField.h"
#include "mir/stats/Statistics.h"
#include "mir/stats/detail/Counter.h"
#include "mir/util/Exceptions.h"


namespace mir::stats::statistics {


/// Generic statistics on a MIRField
template <typename STATS>
class StatisticsT : public Statistics, detail::Counter, STATS {
public:
    // -- Exceptions
    // None

    // -- Constructors

    StatisticsT(const param::MIRParametrisation& parametrisation) :
        Statistics(parametrisation), Counter(parametrisation) {}

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void execute(const data::MIRField& field) override {
        Counter::reset(field);
        STATS::reset();

        ASSERT(field.dimensions() == 1);
        for (auto& value : field.values(0)) {
            if (count(value)) {
                STATS::operator()(value);
            }
        }
    }

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void print(std::ostream& out) const override {
        out << "Statistics[";
        Counter::print(out);
        out << ",";
        STATS::print(out);
        out << "]";
    }

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::stats::statistics
