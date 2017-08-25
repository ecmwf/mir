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

#include <cmath>
#include <complex>
#include "eckit/exception/Exceptions.h"
#include "eckit/types/FloatCompare.h"


namespace mir {
namespace data {
namespace dimension {


namespace detail {


// Angles in degrees [0,360[/[-180,180] or radians [0,2π[/[-π,π]
enum { DEGREE, RADIAN };
enum { ASYMMETRIC, SYMMETRIC };
typedef std::complex<double> complex_t;


// Generic types

template< int SCALE, int SYMMETRY >
struct NormaliseAngle {
    NormaliseAngle() {
        // ensure specialisation
        NOTIMP;
    }
    double normalise(double a) {
        while (a >= MIN + GLOBE) {
            a -= GLOBE;
        }
        while (a < MIN ) {
            a += GLOBE;
        }
        return a;
    }
    const double GLOBE;
    const double MIN;
};

template< int SCALE >
struct ConvertAngle {
    static double toAngle(const complex_t&) {
        // ensure specialisation
        NOTIMP;
    }
    static complex_t toComplex(const double&) {
        // ensure specialisation
        NOTIMP;
    }
};


// Specialised types

template<> NormaliseAngle< DEGREE, ASYMMETRIC >::NormaliseAngle() : GLOBE(360.),     MIN(   0.) {}
template<> NormaliseAngle< DEGREE, SYMMETRIC  >::NormaliseAngle() : GLOBE(360.),     MIN(-180.) {}
template<> NormaliseAngle< RADIAN, ASYMMETRIC >::NormaliseAngle() : GLOBE(M_PI * 2), MIN(   0.) {}
template<> NormaliseAngle< RADIAN, SYMMETRIC  >::NormaliseAngle() : GLOBE(M_PI * 2), MIN(-M_PI) {}

template<> double ConvertAngle< RADIAN >::toAngle(const complex_t& c) {
    if ( eckit::types::is_approximately_equal(c.real(), 0.) &&
         eckit::types::is_approximately_equal(c.imag(), 0.) ) {
        return 0.;
    }
    return std::arg(c);
}

template<> complex_t ConvertAngle< RADIAN >::toComplex(const double& a) {
    return std::polar(1., a);
}

template<> double ConvertAngle< DEGREE >::toAngle(const complex_t& c) {
    return ConvertAngle<RADIAN>::toAngle(c) * (M_1_PI * 180.);
}

template<> complex_t ConvertAngle< DEGREE >::toComplex(const double& a) {
    return ConvertAngle<RADIAN>::toComplex(a * (M_PI / 180.));
}


}  // namespace detail


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


template< int SCALE, int SYMMETRY >
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

