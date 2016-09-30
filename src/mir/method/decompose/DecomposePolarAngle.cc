/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/decompose/DecomposePolarAngle.h"

#include "mir/data/FieldInfo.h"
#include "mir/util/Angles.h"


namespace mir {
namespace method {
namespace decompose {


namespace {
static DecomposeChoice<DecomposePolarAngle<data::FieldInfo::CYLINDRICAL_ANGLE_DEGREES_ASSYMMETRIC> > __decomposePolarAngleDegreesAssymmetric( "PolarAngleDegreesAssymmetric" );
static DecomposeChoice<DecomposePolarAngle<data::FieldInfo::CYLINDRICAL_ANGLE_DEGREES_SYMMETRIC>   > __decomposePolarAngleDegreesSymmetric(   "PolarAngleDegreesSymmetric"   );
static DecomposeChoice<DecomposePolarAngle<data::FieldInfo::CYLINDRICAL_ANGLE_RADIANS_ASSYMMETRIC> > __decomposePolarAngleRadiansAssymmetric( "PolarAngleRadiansAssymmetric" );
static DecomposeChoice<DecomposePolarAngle<data::FieldInfo::CYLINDRICAL_ANGLE_RADIANS_SYMMETRIC>   > __decomposePolarAngleRadiansSymmetric(   "PolarAngleRadiansSymmetric"   );
}


template<>
DecomposePolarAngle<data::FieldInfo::CYLINDRICAL_ANGLE_DEGREES_ASSYMMETRIC>::DecomposePolarAngle() :
    Decompose(),
    fp_angle2xy_(&util::angles::convert_degrees_to_complex<double>),
    fp_xy2angle_(&util::angles::convert_complex_to_degrees<double>),
    fp_normalize_(&util::angles::between_0_and_360) {
}


template<>
DecomposePolarAngle<data::FieldInfo::CYLINDRICAL_ANGLE_DEGREES_SYMMETRIC>::DecomposePolarAngle() :
    Decompose(),
    fp_angle2xy_(&util::angles::convert_degrees_to_complex<double>),
    fp_xy2angle_(&util::angles::convert_complex_to_degrees<double>),
    fp_normalize_(&util::angles::between_m180_and_p180) {
}


template<>
DecomposePolarAngle<data::FieldInfo::CYLINDRICAL_ANGLE_RADIANS_ASSYMMETRIC>::DecomposePolarAngle() :
    Decompose(),
    fp_angle2xy_(&util::angles::convert_radians_to_complex<double>),
    fp_xy2angle_(&util::angles::convert_complex_to_radians<double>),
    fp_normalize_(&util::angles::between_0_and_2PI) {
}


template<>
DecomposePolarAngle<data::FieldInfo::CYLINDRICAL_ANGLE_RADIANS_SYMMETRIC>::DecomposePolarAngle() :
    Decompose(),
    fp_angle2xy_(&util::angles::convert_radians_to_complex<double>),
    fp_xy2angle_(&util::angles::convert_complex_to_radians<double>),
    fp_normalize_(&util::angles::between_mPI_and_pPI) {
}


template<int FIELDINFO_COMPONENT>
void DecomposePolarAngle<FIELDINFO_COMPONENT>::decompose(const WeightMatrix::Matrix& matrixIn, WeightMatrix::Matrix& matrixOut) const {
    ASSERT(matrixIn.cols() == 1);
    matrixOut.resize(matrixIn.rows(), 2);  // allocates memory, not initialised

    std::complex<double> xy;
    for (WeightMatrix::Size i = 0; i < matrixIn.size(); ++i) {
        xy = (*fp_angle2xy_)(matrixIn[i]);
        matrixOut(i, 0) = xy.real();
        matrixOut(i, 1) = xy.imag();
    }
}


template<int FIELDINFO_COMPONENT>
void DecomposePolarAngle<FIELDINFO_COMPONENT>::recompose(const WeightMatrix::Matrix& matrixIn, WeightMatrix::Matrix& matrixOut) const {
    ASSERT(matrixIn.cols() == 2);
    matrixOut.resize(matrixIn.rows(), 1);

    std::complex<double> xy;
    double th;
    for (WeightMatrix::Size i = 0; i < matrixIn.rows(); ++i) {
        xy = std::complex<double>(matrixIn(i, 0), matrixIn(i, 1));
        th = (*fp_xy2angle_)(xy);
        matrixOut[i] = fp_normalize_(th);
    }
}


}  // namespace decompose
}  // namespace method
}  // namespace mir

