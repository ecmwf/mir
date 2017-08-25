/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/data/dimension/Dimension1DAngleT.h"


namespace mir {
namespace data {
namespace dimension {


static DimensionChoice< Dimension1DAngleT< detail::DEGREE, detail::ASYMMETRIC > > __dimension1("1d.angle.degree.asymmetric");
static DimensionChoice< Dimension1DAngleT< detail::DEGREE, detail::SYMMETRIC  > > __dimension2("1d.angle.degree.symmetric");
static DimensionChoice< Dimension1DAngleT< detail::RADIAN, detail::ASYMMETRIC > > __dimension3("1d.angle.radian.asymmetric");
static DimensionChoice< Dimension1DAngleT< detail::RADIAN, detail::SYMMETRIC  > > __dimension4("1d.angle.radian.symmetric");


template< int SCALE, int SYMMETRY >
Dimension1DAngleT< SCALE, SYMMETRY >::Dimension1DAngleT() : Dimension() {
}


template< int SCALE, int SYMMETRY >
void Dimension1DAngleT< SCALE, SYMMETRY >::linearise(const Dimension::Matrix& matrixIn, Dimension::Matrix& matrixOut, double missingValue) const {
    ASSERT(matrixIn.cols() == 1);
    matrixOut.resize(matrixIn.rows(), 2);  // allocates memory, not initialised

    typedef detail::ConvertAngle< SCALE > conv;

    detail::complex_t xy;
    for (Matrix::Size i = 0; i < matrixIn.size(); ++i) {
        if (matrixIn(i, 0) == missingValue) {
            matrixOut(i, 0) = matrixOut(i, 1) = missingValue;
        } else {
            xy = conv::toComplex(matrixIn[i]);
            matrixOut(i, 0) = xy.real();
            matrixOut(i, 1) = xy.imag();
        }
    }
}


template<int SCALE, int SYMMETRY>
void Dimension1DAngleT< SCALE, SYMMETRY >::unlinearise(const Dimension::Matrix& matrixIn, Dimension::Matrix& matrixOut, double missingValue) const {
    ASSERT(matrixIn.rows() == matrixOut.rows());
    ASSERT(matrixIn.cols() == 2);
    ASSERT(matrixOut.cols() == 1);

    typedef detail::ConvertAngle< SCALE > conv;
    detail::NormaliseAngle<SCALE, SYMMETRY> norm;

    detail::complex_t xy;
    double th;
    for (Matrix::Size i = 0; i < matrixIn.rows(); ++i) {
        if (matrixIn(i, 0) == missingValue || matrixIn(i, 1) == missingValue) {
            matrixOut[i] = missingValue;
        } else {
            xy = detail::complex_t(matrixIn(i, 0), matrixIn(i, 1));
            th = conv::toAngle(xy);
            matrixOut[i] = norm.normalise(th);
        }
    }
}


}  // namespace dimension
}  // namespace data
}  // namespace mir

