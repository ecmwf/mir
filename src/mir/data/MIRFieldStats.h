// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>

#include "mir/util/Types.h"


namespace mir::data {


class MIRFieldStats {
public:
    MIRFieldStats();
    MIRFieldStats(const MIRValuesVector&, size_t missing);

    double maximum() const;
    double minimum() const;

private:
    size_t count_;
    size_t missing_;
    double min_;
    double max_;
    double mean_;
    double sqsum_;
    double stdev_;

    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& s, const MIRFieldStats& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::data
