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
static DecomposeBuilder<DecomposePolarAngle<data::FieldInfo::CYLINDRICAL_ANGLE_DEGREES_ASSYMMETRIC> > __decomposePolarAngleDegreesAssymmetric( "DecomposePolarAngleDegreesAssymmetric");
static DecomposeBuilder<DecomposePolarAngle<data::FieldInfo::CYLINDRICAL_ANGLE_DEGREES_SYMMETRIC>   > __decomposePolarAngleDegreesSymmetric(   "DecomposePolarAngleDegreesSymmetric");
static DecomposeBuilder<DecomposePolarAngle<data::FieldInfo::CYLINDRICAL_ANGLE_RADIANS_ASSYMMETRIC> > __decomposePolarAngleRadiansAssymmetric( "DecomposePolarAngleRadiansAssymmetric");
static DecomposeBuilder<DecomposePolarAngle<data::FieldInfo::CYLINDRICAL_ANGLE_RADIANS_SYMMETRIC>   > __decomposePolarAngleRadiansSymmetric(   "DecomposePolarAngleRadiansSymmetric");
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


template< int FIELDINFO_COMPONENT >
void DecomposePolarAngle<FIELDINFO_COMPONENT>::decompose(WeightMatrix::Vector& v) {

    matrix_.resize(v.size(), 2);  // allocates memory, not initialised

    std::complex<double> xy;
    for (WeightMatrix::Size i = 0; i < v.size(); ++i) {
        xy = (*fp_angle2xy_)(v[i]);
        matrix_(i, 0) = xy.real();
        matrix_(i, 1) = xy.imag();
    }
}


template< int FIELDINFO_COMPONENT >
void DecomposePolarAngle<FIELDINFO_COMPONENT>::recompose(WeightMatrix::Vector& v) const {
    ASSERT(v.size() == matrix_.rows());

    std::complex<double> xy;
    double th;
    for (WeightMatrix::Size i = 0; i < matrix_.rows(); ++i) {
        xy = std::complex<double>(matrix_(i, 0), matrix_(i, 1));
        th = (*fp_xy2angle_)(xy);
        v[i] = fp_normalize_(th);
    }
}


}  // namespace decompose
}  // namespace method
}  // namespace mir

