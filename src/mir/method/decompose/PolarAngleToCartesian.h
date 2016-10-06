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

#include "mir/method/decompose/Decompose.h"

#include "mir/util/Angles.h"


namespace mir {
namespace method {
namespace decompose {


template< int FIELDINFO_COMPONENT >
class PolarAngleToCartesian : public Decompose {
public:

    // -- Exceptions
    // None

    // -- Constructors

    PolarAngleToCartesian() {
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

    void decompose(const WeightMatrix::Matrix&, WeightMatrix::Matrix&, double missingValue=std::numeric_limits<double>::quiet_NaN()) const;

    void recompose(const WeightMatrix::Matrix&, WeightMatrix::Matrix&, double missingValue=std::numeric_limits<double>::quiet_NaN()) const;

    inline std::complex<double> decompose(const double&, double missingValue=std::numeric_limits<double>::quiet_NaN()) const;

    inline double recompose(const std::complex<double>&, double missingValue=std::numeric_limits<double>::quiet_NaN()) const;

    inline double normalize(const double&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Types

    typedef double (*fp_normalize_t)(double);
    typedef double (*fp_cartesian_to_angle_t)(const std::complex<double>&);
    typedef std::complex<double> (*fp_angle_to_cartesian_t)(const double&);

    // -- Members

    const fp_angle_to_cartesian_t fp_angle_to_cartesian_;
    const fp_cartesian_to_angle_t fp_cartesian_to_angle_;
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


}  // namespace decompose
}  // namespace method
}  // namespace mir


#endif

