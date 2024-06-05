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

#include <memory>
#include <vector>

#include "eckit/filesystem/PathName.h"
#include "mir/method/Method.h"


namespace eckit {
class JSON;
}

namespace mir::method {
namespace nonlinear {
class NonLinear;
}
namespace solver {
class Solver;
}
}  // namespace mir::method


namespace mir::method {


class Matrix final : public Method {
public:
    // -- Constructors

    explicit Matrix(const param::MIRParametrisation&);

private:
    // -- Members

    eckit::PathName matrix_;
    std::unique_ptr<const solver::Solver> solver_;
    std::vector<std::unique_ptr<const nonlinear::NonLinear>> nonLinear_;

    std::string matrix_loader_;
    std::string vector_space_;

    // -- Methods

    void json(eckit::JSON&) const;
    void solve(context::Context&, const repres::Representation& in, const repres::Representation& out) const;

    // -- Overridden methods

    // From Method
    void execute(context::Context&, const repres::Representation& in, const repres::Representation& out) const override;

    int version() const override { return 0; }

    void hash(eckit::MD5&) const override;
    bool sameAs(const Method&) const override;
    void print(std::ostream&) const override;

    bool canCrop() const override { return false; }
    bool hasCropping() const override { return false; }
    void setCropping(const util::BoundingBox&) override;
    const util::BoundingBox& getCropping() const override;

    // -- Friends

    friend eckit::JSON& operator<<(eckit::JSON& s, const Matrix& o) {
        o.json(s);
        return s;
    }
};


}  // namespace mir::method
