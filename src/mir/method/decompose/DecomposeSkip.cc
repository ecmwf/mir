/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/decompose/DecomposeSkip.h"


namespace mir {
namespace method {
namespace decompose {


namespace {
static DecomposeBuilder<DecomposeSkip> __decomposeSkip("DecomposeSkip");
}


DecomposeSkip::DecomposeSkip() {
}


void DecomposeSkip::decompose(const WeightMatrix::Matrix& matrixIn, WeightMatrix::Matrix& matrixOut) const {

    // shallow-copy input to output
    using eckit::linalg::Scalar;
    Scalar* data = const_cast<Scalar*>(matrixIn.data());
    WeightMatrix::Matrix shallow(data, matrixIn.rows(), matrixIn.cols());

    matrixOut.swap(shallow);
}


void DecomposeSkip::recompose(const WeightMatrix::Matrix& matrixIn, WeightMatrix::Matrix& matrixOut) const {

    // shallow-copy input to output
    using eckit::linalg::Scalar;
    Scalar* data = const_cast<Scalar*>(matrixIn.data());
    WeightMatrix::Matrix shallow(data, matrixIn.rows(), matrixIn.cols());

    matrixOut.swap(shallow);
}


}  // namespace decompose
}  // namespace method
}  // namespace mir

