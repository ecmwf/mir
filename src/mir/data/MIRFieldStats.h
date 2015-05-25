/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Baudouin Raoult
/// @date   May 2015

#ifndef MIRFieldStats_H
#define MIRFieldStats_H

#include <iosfwd>
#include <vector>

namespace mir {
namespace data {

struct MIRFieldStats {

    double min;
    double max;
    double mean;
    double sqsum;
    double stdev;

    MIRFieldStats();
    MIRFieldStats(const std::vector<double>& vs);

    void compute(const std::vector<double>& vs);

    void print(std::ostream& s) const;

    friend std::ostream &operator<<(std::ostream &s, const MIRFieldStats &p) {
        p.print(s);
        return s;
    }
};

}  // namespace data
}  // namespace mir

#endif

