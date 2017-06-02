/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_method_decompose_PolarAngleToCartesian_h
#define mir_method_decompose_PolarAngleToCartesian_h

#include "mir/method/decompose/DecomposeToCartesian.h"

#include "mir/data/FieldInfo.h"
#include "mir/util/Angles.h"


namespace mir {
namespace method {
namespace decompose {


template< int FIELDINFO_COMPONENT >
class PolarAngleToCartesian : public DecomposeToCartesian {
public:

    // -- Exceptions
    // None

    // -- Constructors

    PolarAngleToCartesian(double missingValue=std::numeric_limits<double>::quiet_NaN()) :
        DecomposeToCartesian(missingValue),
        fp_angle_to_complex_(NULL),
        fp_complex_to_angle_(NULL),
        fp_normalize_(NULL) {
        ASSERT(fp_angle_to_complex_);
        ASSERT(fp_complex_to_angle_);
        ASSERT(fp_normalize_);

        // ensure constructor is specialized
        NOTIMP;
    }

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void decompose(const WeightMatrix::Matrix&, WeightMatrix::Matrix&) const;

    void recompose(const WeightMatrix::Matrix&, WeightMatrix::Matrix&) const;

    inline std::complex<double> decomposeValue(const double& angle) const {
        if (isMissing_(angle)) {
            return std::complex<double>(isMissing_.missingValue_, isMissing_.missingValue_);
        }
        return (*fp_angle_to_complex_)(angle);
    }

    inline double recomposeValue(const std::complex<double>& complex) const {
        if (isMissing_(complex.real()) || isMissing_(complex.imag())) {
            return isMissing_.missingValue_;
        }
        return (*fp_normalize_)((*fp_complex_to_angle_)(complex));
    }

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Types

    typedef double (*fp_normalize_t)(double);
    typedef double (*fp_complex_to_angle_t)(const std::complex<double>&);
    typedef std::complex<double> (*fp_angle_to_complex_t)(const double&);

    // -- Members

    const fp_angle_to_complex_t fp_angle_to_complex_;
    const fp_complex_to_angle_t fp_complex_to_angle_;
    const fp_normalize_t fp_normalize_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


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


}  // namespace decompose
}  // namespace method
}  // namespace mir


#endif

