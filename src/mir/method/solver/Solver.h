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
class JSON;
class MD5;
}  // namespace eckit

namespace mir::param {
class MIRParametrisation;
}  // namespace mir::param


namespace mir::method::solver {


/// Solve linear system (B = W A)
class Solver {
public:
    explicit Solver(const param::MIRParametrisation&) {}

    Solver(const Solver&) = delete;
    Solver(Solver&&)      = delete;

    virtual ~Solver() = default;

    void operator=(const Solver&) = delete;
    void operator=(Solver&&)      = delete;

    virtual void solve(const DenseMatrix& A, const WeightMatrix& W, DenseMatrix& B,
                       const double& missingValue) const = 0;

    virtual bool sameAs(const Solver&) const = 0;
    virtual void hash(eckit::MD5&) const     = 0;
    virtual void json(eckit::JSON&) const    = 0;

private:
    virtual void print(std::ostream&) const = 0;

    friend std::ostream& operator<<(std::ostream& s, const Solver& p) {
        p.print(s);
        return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const Solver& p) {
        p.json(s);
        return s;
    }
};


}  // namespace mir::method::solver
