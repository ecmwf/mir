// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/data/space/SpaceLinear.h"


namespace mir::data::space {


SpaceLinear::SpaceLinear() = default;


void SpaceLinear::linearise(const Space::Matrix& matrixIn, Space::Matrix& matrixOut, double /*missingValue*/) const {

    // shallow-copy input to output
    auto* data = const_cast<eckit::linalg::Scalar*>(matrixIn.data());
    Matrix shallow(data, matrixIn.rows(), matrixIn.cols());

    matrixOut.swap(shallow);
}


void SpaceLinear::unlinearise(const Space::Matrix& matrixIn, Space::Matrix& matrixOut, double /*missingValue*/) const {

    // shallow-copy input to output
    auto* data = const_cast<eckit::linalg::Scalar*>(matrixIn.data());
    Matrix shallow(data, matrixIn.rows(), matrixIn.cols());

    matrixOut.swap(shallow);
}


}  // namespace mir::data::space
