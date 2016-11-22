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
/// @author Florian Rathgeber
/// @date May 2015


#ifndef mir_method_WeightMatrix_H
#define mir_method_WeightMatrix_H

#include <sstream>
#include "eckit/linalg/Matrix.h"
#include "eckit/linalg/SparseMatrix.h"


namespace mir {
namespace method {

//----------------------------------------------------------------------------------------------------------------------

class WeightMatrix : public eckit::linalg::SparseMatrix {
    // NOTE: protected inheritance would be better but
    // there are issues interfacing the iterators

public: // types

    typedef eckit::linalg::Triplet Triplet;
    typedef eckit::linalg::Matrix  Matrix;
    typedef eckit::linalg::Vector  Vector;
    typedef eckit::linalg::Size    Size;

public: // methods

    WeightMatrix() {}

    WeightMatrix(Size rows, Size cols) : SparseMatrix(rows, cols) {}

    void setFromTriplets(const std::vector<Triplet>&);

    void multiply(const Vector& values, Vector& result) const;

    void multiply(const Matrix& values, Matrix& result) const;

    void cleanup(const double& pruneEpsilon = 0);

    void validate(const char *when) const;

    using SparseMatrix::rows;
    using SparseMatrix::cols;

    using SparseMatrix::save;
    using SparseMatrix::load;
    using SparseMatrix::setIdentity;
    using SparseMatrix::prune;
    using SparseMatrix::footprint;

    using SparseMatrix::const_iterator;
    using SparseMatrix::iterator;
    using SparseMatrix::begin;
    using SparseMatrix::end;

private: // members

    void print(std::ostream& s) const;

    friend std::ostream& operator<<(std::ostream& out, const WeightMatrix& m) {
        m.print(out);
        return out;
    }

};

//----------------------------------------------------------------------------------------------------------------------


}  // namespace method
}  // namespace mir


#endif
