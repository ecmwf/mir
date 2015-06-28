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

#include "eckit/exception/Exceptions.h"

#include "experimental/eckit/la/LinearAlgebraFactory.h"
#include "experimental/eckit/la/SparseMatrix.h"
#include "experimental/eckit/la/Vector.h"

namespace eckit {
class PathName;
}

namespace mir {
namespace method {


class WeightMatrix {

    typedef eckit::la::SparseMatrix Matrix;

public:
    typedef Matrix::Index Index;
    typedef Matrix::Scalar Scalar;
    typedef eckit::la::Triplet Triplet;

    // TODO: linear algebra backend should depend on parametrisation
    WeightMatrix()
        : matrix_(), la_(eckit::la::LinearAlgebraFactory::get()) {}

    // TODO: linear algebra backend should depend on parametrisation
    WeightMatrix(Index rows, Index cols):
        matrix_(rows, cols), la_(eckit::la::LinearAlgebraFactory::get()) {
    }

    void save(const eckit::PathName &path) const;
    void load(const eckit::PathName &path);


    Index rows() const {
        return matrix_.rows();
    }

    Index cols() const {
        return matrix_.cols();
    }

    // Index innerSize() const {
    //     return matrix_.innerSize();
    // }

    // Index outerSize() const {
    //     return matrix_.outerSize();
    // }

    void setIdentity() {
        matrix_.setIdentity();
    }

    void prune(double value) {
        matrix_.prune(value);
    }

    void multiply(const std::vector<double> &values, std::vector<double> &result) const {

        // FIXME: remove this const cast once Vector provides read-only view
        eckit::la::Vector vi(const_cast<double *>(values.data()), values.size());
        eckit::la::Vector vo(result.data(), result.size());

        la_->spmv(matrix_, vi, vo);
    }

    void cleanup();
    void validate(const char *when) const;

    class inner_iterator : public Matrix::InnerIterator {
    public:
        inner_iterator( WeightMatrix &m, Index outer) :
            Matrix::InnerIterator(m.matrix_, outer) {}
    };

    class inner_const_iterator : public Matrix::InnerIterator {
    public:
        // FIXME: Remove const_cast once SparseMatrix provides const iterator
        inner_const_iterator(const WeightMatrix &m, Index outer) :
            Matrix::InnerIterator(const_cast<Matrix&>(m.matrix_), outer) {}
    };

    void setFromTriplets(const std::vector<Triplet>& triplets) {
        matrix_.setFromTriplets(triplets);
    }

private:

    Matrix matrix_;
    const eckit::la::LinearAlgebraBase* la_;
};



}  // namespace method
}  // namespace mir

#endif
