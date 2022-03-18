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
#include <vector>

#include "eckit/linalg/Matrix.h"
#include "eckit/linalg/SparseMatrix.h"


namespace mir {
namespace method {


class WeightMatrix : public eckit::linalg::SparseMatrix {

public:  // types
    using Triplet = eckit::linalg::Triplet;
    using Matrix  = eckit::linalg::Matrix;
    using Vector  = eckit::linalg::Vector;
    using Scalar  = eckit::linalg::Scalar;
    using Size    = eckit::linalg::Size;

public:  // constructors
    using SparseMatrix::SparseMatrix;
    WeightMatrix(const eckit::PathName&);

public:  // methods
    void cleanup(const double& pruneEpsilon = 0);

    void validate(const char* when) const;

    using SparseMatrix::cols;
    using SparseMatrix::rows;

    using SparseMatrix::footprint;
    using SparseMatrix::load;
    using SparseMatrix::prune;
    using SparseMatrix::save;
    using SparseMatrix::setIdentity;

    using SparseMatrix::begin;
    using SparseMatrix::const_iterator;
    using SparseMatrix::end;
    using SparseMatrix::iterator;

private:  // members
    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& out, const WeightMatrix& m) {
        m.print(out);
        return out;
    }
};


}  // namespace method
}  // namespace mir
