/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/decompose/PolarAngleToCartesian.h"

#include "mir/data/FieldInfo.h"
#include "mir/util/Angles.h"


namespace mir {
namespace method {
namespace decompose {


namespace {
static DecomposeToCartesianChoice<PolarAngleToCartesian<data::FieldInfo::CYLINDRICAL_ANGLE_DEGREES_ASYMMETRIC> > __polarAngleDegreesAsymmetricToCartesian( "PolarAngleDegreesAsymmetricToCartesian" );
static DecomposeToCartesianChoice<PolarAngleToCartesian<data::FieldInfo::CYLINDRICAL_ANGLE_DEGREES_SYMMETRIC>  > __polarAngleDegreesSymmetricToCartesian(  "PolarAngleDegreesSymmetricToCartesian"  );
static DecomposeToCartesianChoice<PolarAngleToCartesian<data::FieldInfo::CYLINDRICAL_ANGLE_RADIANS_ASYMMETRIC> > __polarAngleRadiansAsymmetricToCartesian( "PolarAngleRadiansAsymmetricToCartesian" );
static DecomposeToCartesianChoice<PolarAngleToCartesian<data::FieldInfo::CYLINDRICAL_ANGLE_RADIANS_SYMMETRIC>  > __polarAngleRadiansSymmetricToCartesian(  "PolarAngleRadiansSymmetricToCartesian"  );
}


template<>
PolarAngleToCartesian<data::FieldInfo::CYLINDRICAL_ANGLE_DEGREES_ASYMMETRIC>::PolarAngleToCartesian(double missingValue) :
    DecomposeToCartesian(missingValue),
    fp_angle_to_complex_(&util::angles::convert_degrees_to_complex<double>),
    fp_complex_to_angle_(&util::angles::convert_complex_to_degrees<double>),
    fp_normalize_(&util::angles::between_0_and_360) {
}


template<>
PolarAngleToCartesian<data::FieldInfo::CYLINDRICAL_ANGLE_DEGREES_SYMMETRIC>::PolarAngleToCartesian(double missingValue) :
    DecomposeToCartesian(missingValue),
    fp_angle_to_complex_(&util::angles::convert_degrees_to_complex<double>),
    fp_complex_to_angle_(&util::angles::convert_complex_to_degrees<double>),
    fp_normalize_(&util::angles::between_m180_and_p180) {
}


template<>
PolarAngleToCartesian<data::FieldInfo::CYLINDRICAL_ANGLE_RADIANS_ASYMMETRIC>::PolarAngleToCartesian(double missingValue) :
    DecomposeToCartesian(missingValue),
    fp_angle_to_complex_(&util::angles::convert_radians_to_complex<double>),
    fp_complex_to_angle_(&util::angles::convert_complex_to_radians<double>),
    fp_normalize_(&util::angles::between_0_and_2PI) {
}


template<>
PolarAngleToCartesian<data::FieldInfo::CYLINDRICAL_ANGLE_RADIANS_SYMMETRIC>::PolarAngleToCartesian(double missingValue) :
    DecomposeToCartesian(missingValue),
    fp_angle_to_complex_(&util::angles::convert_radians_to_complex<double>),
    fp_complex_to_angle_(&util::angles::convert_complex_to_radians<double>),
    fp_normalize_(&util::angles::between_mPI_and_pPI) {
}


template<int FIELDINFO_COMPONENT>
void PolarAngleToCartesian<FIELDINFO_COMPONENT>::decompose(const WeightMatrix::Matrix& matrixIn, WeightMatrix::Matrix& matrixOut) const {
    ASSERT(matrixIn.cols() == 1);
    matrixOut.resize(matrixIn.rows(), 2);  // allocates memory, not initialised

    // check if a missingValue is defined
    const bool hasMissing(isMissing_.missingValueDefined_);

    std::complex<double> xy;
    for (WeightMatrix::Size i = 0; i < matrixIn.size(); ++i) {
        if (hasMissing && (isMissing_(matrixIn(i, 0)))) {
            matrixOut(i, 0) = isMissing_.missingValue_;
            matrixOut(i, 1) = isMissing_.missingValue_;
        } else {
            xy = (*fp_angle_to_complex_)(matrixIn[i]);
            matrixOut(i, 0) = xy.real();
            matrixOut(i, 1) = xy.imag();
        }
    }
}


template<int FIELDINFO_COMPONENT>
void PolarAngleToCartesian<FIELDINFO_COMPONENT>::recompose(const WeightMatrix::Matrix& matrixIn, WeightMatrix::Matrix& matrixOut) const {
    ASSERT(matrixIn.rows() == matrixOut.rows());
    ASSERT(matrixIn.cols() == 2);
    ASSERT(matrixOut.cols() == 1);

    // check if a missingValue is defined
    const bool hasMissing(isMissing_.missingValueDefined_);

    std::complex<double> xy;
    double th;
    for (WeightMatrix::Size i = 0; i < matrixIn.rows(); ++i) {
        if (hasMissing && (isMissing_(matrixIn(i, 0)) || isMissing_(matrixIn(i, 1)))) {
            matrixOut[i] = isMissing_.missingValue_;
        } else {
            xy = std::complex<double>(matrixIn(i, 0), matrixIn(i, 1));
            th = (*fp_complex_to_angle_)(xy);
            matrixOut[i] = fp_normalize_(th);
        }
    }
}


}  // namespace decompose
}  // namespace method
}  // namespace mir

