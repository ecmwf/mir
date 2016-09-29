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

#include "mir/util/Angles.h"


namespace mir {
namespace method {
namespace decompose {


using namespace util::angles;


namespace {


// FIXME temporary: this is certainly not proper
static bool degrees   = true;
static bool symmetric = false;


namespace {
static DecomposeBuilder<DecomposePolarAngle> __decomposePolarAngle("DecomposePolarAngle");
}


}  // (anonymous namespace)


DecomposePolarAngle::DecomposePolarAngle() :
    Decompose(),
    fp_angle2xy_(degrees? &util::angles::convert_degrees_to_complex<double> : &util::angles::convert_radians_to_complex<double>),
    fp_xy2angle_(degrees? &util::angles::convert_complex_to_degrees<double> : &util::angles::convert_complex_to_radians<double>),
    fp_normalize_(degrees &&  symmetric? &util::angles::between_m180_and_p180
                : degrees && !symmetric? &util::angles::between_0_and_360
                : symmetric?             &util::angles::between_mPI_and_pPI
                :                        &util::angles::between_0_and_2PI) {
    ASSERT(fp_angle2xy_);
    ASSERT(fp_xy2angle_);
    ASSERT(fp_normalize_);
}


void DecomposePolarAngle::decompose(WeightMatrix::Vector& v) {
    xy_.resize(v.size(), 2);
    for (WeightMatrix::Size i = 0; i < v.size(); ++i) {
        std::complex<double> xy = (*fp_angle2xy_)(v[i]);
        xy_(i, 0) = xy.real();
        xy_(i, 1) = xy.imag();
    }
}


void DecomposePolarAngle::recompose(WeightMatrix::Vector& v) const {
    ASSERT(v.size() == xy_.rows());
    for (WeightMatrix::Size i = 0; i < xy_.rows(); ++i) {
        v[i] = fp_normalize_( (*fp_xy2angle_)(std::complex<double>(xy_(i, 0), xy_(i, 1))) );
    }
}


}  // namespace decompose
}  // namespace method
}  // namespace mir

