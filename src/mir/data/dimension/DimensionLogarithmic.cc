/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/data/dimension/DimensionLogarithmic.h"

#include <cmath>
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace data {
namespace dimension {


static DimensionChoice<DimensionLogarithmic> __dimension("1d.logarithmic");


DimensionLogarithmic::DimensionLogarithmic() : Dimension() {
}


void DimensionLogarithmic::linearise(const Dimension::Matrix& matrixIn, Dimension::Matrix& matrixOut, double missingValue) const {
    matrixOut.resize(matrixIn.rows(), matrixIn.cols());  // allocates memory, not initialised

    for (Matrix::Size i = 0; i < matrixIn.size(); ++i) {
        if (matrixIn[i] == missingValue) {
            matrixOut[i] = missingValue;
        } else {
            matrixOut[i] = std::exp(matrixIn[i]);
        }
    }
}


void DimensionLogarithmic::unlinearise(const Dimension::Matrix& matrixIn, Dimension::Matrix& matrixOut, double missingValue) const {
    ASSERT(matrixIn.rows() == matrixOut.rows());

    for (Matrix::Size i = 0; i < matrixIn.size(); ++i) {
        if (matrixIn[i] == missingValue) {
            matrixOut[i] = missingValue;
        } else {
            ASSERT(matrixIn[i] > 0.);
            matrixOut[i] = std::log(matrixIn[i]);
        }
    }
}


}  // namespace decompose
}  // namespace method
}  // namespace mir

