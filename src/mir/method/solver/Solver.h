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
#include <string>

#include "mir/method/MethodWeighted.h"


namespace eckit {
class MD5;
}

namespace mir {
namespace param {
class MIRParametrisation;
}
}  // namespace mir


namespace mir {
namespace method {
namespace solver {


class Solver {
public:
    Solver(const param::MIRParametrisation&) {}

    Solver(const Solver&) = delete;
    void operator=(const Solver&) = delete;

    virtual ~Solver() = default;

    /// Update interpolation linear system (B = W A) to account for non-linearities
    virtual void solve(const MethodWeighted::Matrix& A, const MethodWeighted::WeightMatrix& W,
                       MethodWeighted::Matrix& B, const double& missingValue) const = 0;

    virtual bool sameAs(const Solver&) const = 0;

    virtual void hash(eckit::MD5&) const = 0;

private:
    virtual void print(std::ostream&) const = 0;

    friend std::ostream& operator<<(std::ostream& s, const Solver& p) {
        p.print(s);
        return s;
    }
};


}  // namespace solver
}  // namespace method
}  // namespace mir
