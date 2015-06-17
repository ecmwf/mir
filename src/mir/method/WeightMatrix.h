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

#include "eckit/eckit_config.h"

// #ifdef ECKIT_HAVE_EIGEN
// #ifdef ECKIT_HAVE_ARMADILLO

#include "eckit/maths/Eigen.h"
#include "eckit/exception/Exceptions.h"

namespace eckit {
class PathName;
}

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

    void save(const eckit::PathName &path) const;
    void load(const eckit::PathName &path);


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

    void cleanup();
    void validate(const char *when) const;


private:
    // Solve the const-ness issues in eigen

    class _inner_iterator {
      protected:
        Matrix::InnerIterator it_;
      public:

        _inner_iterator(const WeightMatrix &m, Index outer):
            it_(m.matrix_, outer) {
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

        double operator*() const {
            return it_.value();
        }

    };

public:
    class inner_iterator : public _inner_iterator {
      public:

        inner_iterator( WeightMatrix &m, Index outer) :
            _inner_iterator(m, outer) {
        }

        double &operator*() {
            return it_.valueRef();
        }
    };

    class inner_const_iterator : public _inner_iterator {
      public:

        inner_const_iterator( const WeightMatrix &m, Index outer) :
            _inner_iterator(m, outer) {
        }

    };

    void setFromTriplets(const std::vector<Triplet>& triplets) {
        matrix_.setFromTriplets(triplets.begin(), triplets.end());
    }

  private:

    Matrix matrix_;



};



}  // namespace method
}  // namespace mir

#endif
