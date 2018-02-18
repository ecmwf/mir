/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/data/dimension/DimensionLinear.h"


namespace mir {
namespace data {
namespace dimension {


DimensionLinear::DimensionLinear() : Dimension() {
}


void DimensionLinear::linearise(const Dimension::Matrix& matrixIn, Dimension::Matrix& matrixOut, double) const {

    // shallow-copy input to output
    using eckit::linalg::Scalar;
    Scalar* data = const_cast<Scalar*>(matrixIn.data());
    Matrix shallow(data, matrixIn.rows(), matrixIn.cols());

    matrixOut.swap(shallow);
}


void DimensionLinear::unlinearise(const Dimension::Matrix& matrixIn, Dimension::Matrix& matrixOut, double) const {

    // shallow-copy input to output
    using eckit::linalg::Scalar;
    Scalar* data = const_cast<Scalar*>(matrixIn.data());
    Matrix shallow(data, matrixIn.rows(), matrixIn.cols());

    matrixOut.swap(shallow);
}


}  // namespace decompose
}  // namespace method
}  // namespace mir

