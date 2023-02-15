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


#include "mir/data/space/SpaceLogarithmic.h"

#include <cmath>

#include "mir/util/Exceptions.h"


namespace mir::data::space {


static const SpaceChoice<SpaceLogarithmic> __space("1d-logarithmic");


SpaceLogarithmic::SpaceLogarithmic() = default;


void SpaceLogarithmic::linearise(const Space::Matrix& matrixIn, Space::Matrix& matrixOut, double missingValue) const {
    matrixOut.resize(matrixIn.rows(), matrixIn.cols());  // allocates memory, not initialised

    for (Matrix::Size i = 0; i < matrixIn.size(); ++i) {
        if (matrixIn[i] == missingValue) {
            matrixOut[i] = missingValue;
        }
        else {
            matrixOut[i] = std::exp(matrixIn[i]);
        }
    }
}


void SpaceLogarithmic::unlinearise(const Space::Matrix& matrixIn, Space::Matrix& matrixOut, double missingValue) const {
    ASSERT(matrixIn.rows() == matrixOut.rows());

    for (Matrix::Size i = 0; i < matrixIn.size(); ++i) {
        if (matrixIn[i] == missingValue) {
            matrixOut[i] = missingValue;
        }
        else {
            ASSERT(matrixIn[i] > 0.);
            matrixOut[i] = std::log(matrixIn[i]);
        }
    }
}


size_t SpaceLogarithmic::dimensions() const {
    return 1;
}


}  // namespace mir::data::space
