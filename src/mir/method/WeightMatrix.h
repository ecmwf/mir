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


namespace eckit {
class PathName;
}


namespace mir {
namespace method {


class WeightMatrix {

  public: // types

    typedef eckit::linalg::SparseMatrix  SparseMatrix;
    typedef eckit::linalg::Matrix        Matrix;
    typedef SparseMatrix::Size           Size;
    typedef std::vector<double>          Vector;

  public: // methods

    typedef eckit::linalg::Triplet Triplet;

    WeightMatrix();

    WeightMatrix(Size rows, Size cols);

    void save(const eckit::PathName &path) const;

    void load(const eckit::PathName &path);

    Size rows() const {
        return matrix_.rows();
    }

    Size cols() const {
        return matrix_.cols();
    }

    void setFromTriplets(const std::vector<Triplet>& triplets);

    void setIdentity();

    void prune(double value);

    void multiply(const Vector& values, Vector& result) const;

    void multiply(const Matrix& values, Matrix& result) const;

    void cleanup();

    void validate(const char *when) const;

    class inner_iterator : public SparseMatrix::InnerIterator {
    public:
        inner_iterator(WeightMatrix &m, eckit::linalg::Index outer) :
            SparseMatrix::InnerIterator(m.matrix_, outer) {}
    };

    class inner_const_iterator : public SparseMatrix::ConstInnerIterator {
    public:
        inner_const_iterator(const WeightMatrix &m, eckit::linalg::Index outer) :
            SparseMatrix::ConstInnerIterator(m.matrix_, outer) {}
    };

    SparseMatrix& matrix() {
        return matrix_;
    }

  private: // members

    SparseMatrix matrix_;

    void print(std::ostream& s) const;

    friend std::ostream& operator<<(std::ostream& out, const WeightMatrix& e) {
        e.print(out);
        return out;
    }
};


}  // namespace method
}  // namespace mir


#endif
