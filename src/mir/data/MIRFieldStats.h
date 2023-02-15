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
