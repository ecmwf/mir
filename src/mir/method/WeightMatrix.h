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

//----------------------------------------------------------------------------------------------------------------------

class WeightMatrix {

  public: // types

    typedef eckit::linalg::SparseMatrix SparseMatrix;
    typedef eckit::linalg::Matrix       Matrix;
    typedef eckit::linalg::Vector       Vector;
    typedef eckit::linalg::Size         Size;

  public: // methods

    typedef eckit::linalg::Triplet Triplet;

    WeightMatrix();

    WeightMatrix(Size rows, Size cols);

    void save(const eckit::PathName &path) const;

    void load(const eckit::PathName &path);

    void setIdentity();
    void setFromTriplets(const std::vector<Triplet>& triplets);

    Size rows() const {
        return matrix_.rows();
    }

    Size cols() const {
        return matrix_.cols();
    }

    void prune(double value);

    void multiply(const Vector& values, Vector& result) const;

    void multiply(const Matrix& values, Matrix& result) const;

    void cleanup(const double& pruneEpsilon = 0);

    void validate(const char *when) const;

    struct iterator : SparseMatrix::iterator {
        iterator(WeightMatrix& m, Size rowIndex = 0) : SparseMatrix::iterator(m.matrix_, rowIndex) {}
    };

    struct const_iterator : SparseMatrix::const_iterator {
        const_iterator(const WeightMatrix& m, Size rowIndex = 0) : SparseMatrix::const_iterator(m.matrix_, rowIndex) {}
    };

    const_iterator begin(Size rowIndex=0) const { return const_iterator(*this, rowIndex); }
    const_iterator end()                  const { return const_iterator(*this, rows()); }
    const_iterator end(Size rowIndex)     const { return const_iterator(*this, rowIndex+1); }

    iterator       begin(Size rowIndex=0)   { return iterator(*this, rowIndex); }
    iterator       end()                    { return iterator(*this, rows()); }
    iterator       end(Size rowIndex)       { return iterator(*this, rowIndex+1); }

    SparseMatrix& matrix() {
        return matrix_;
    }

    size_t footprint() const;

  private: // members

    SparseMatrix matrix_;

    void print(std::ostream& s) const;

    friend std::ostream& operator<<(std::ostream& out, const WeightMatrix& e) {
        e.print(out);
        return out;
    }
};

//----------------------------------------------------------------------------------------------------------------------


}  // namespace method
}  // namespace mir


#endif
