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
/// @date May 2015

#ifndef mir_method_WeightMatrix_H
#define mir_method_WeightMatrix_H

#include "eckit/maths/Eigen.h"

namespace mir {
namespace method {


class WeightMatrix {

    typedef Eigen::SparseMatrix<double, Eigen::RowMajor> Matrix;

  public:
    typedef Matrix::Index Index;
    typedef Eigen::Triplet<double> Triplet;

    WeightMatrix():
        matrix_() {
    }

    WeightMatrix(Index rows, Index cols):
        matrix_(rows, cols) {
    }

    Index rows() const {
        return matrix_.rows();
    }

    Index cols() const {
        return matrix_.cols();
    }

    Index innerSize() const {
        return matrix_.innerSize();
    }

    Index outerSize() const {
        return matrix_.outerSize();
    }

    void setIdentity() {
        matrix_.setIdentity();
    }

    void prune(double value) {
        matrix_.prune(value);
    }

    void multiply(const std::vector<double> &values, std::vector<double> &result) const {

        // FIXME: remove this const cast
        Eigen::VectorXd::MapType vi = Eigen::VectorXd::Map( const_cast<double *>(&values[0]), cols() );
        Eigen::VectorXd::MapType vo = Eigen::VectorXd::Map( &result[0], rows() );

        vo = matrix_ * vi;
    }

    class InnerIterator {
        Matrix::InnerIterator it_;
        bool const_;
      public:

        InnerIterator(const WeightMatrix &m, Index outer):
            it_(m.matrix_, outer),
            const_(true) {
        }

        InnerIterator( WeightMatrix &m, Index outer) :
            it_(m.matrix_, outer),
            const_(false) {
        }

        operator bool() const {
            return it_;
        }

        void operator++() {
            ++it_;
        }

        Index row() const {
            return it_.row();
        }

        Index col() const {
            return it_.col();
        }

        double value() const {
            return it_.value();
        }

        double &valueRef() {
            ASSERT(!const_);
            return it_.valueRef();
        }
    };

    template<class T>
    void setFromTriplets(T begin, T end) {
        matrix_.setFromTriplets(begin, end);
    }

  private:

    Matrix matrix_;

};



}  // namespace method
}  // namespace mir

#endif
