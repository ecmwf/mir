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


#include "mir/data/space/Space1DAngleT.h"

#include <cmath>
#include <complex>

#include "eckit/types/FloatCompare.h"

#include "mir/util/Angles.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace data {
namespace space {


namespace {


// Angles in degrees [0,360[/[-180,180] or radians [0,2π[/[-π,π]
enum
{
    DEGREE,
    RADIAN
};
enum
{
    ASYMMETRIC,
    SYMMETRIC
};


using complex_t = std::complex<double>;


// Generic types

template <int SCALE, int SYMMETRY>
struct NormaliseAngle {
    NormaliseAngle() {
        // ensure specialisation
        NOTIMP;
    }
    double normalise(double a) const {
        while (a >= MIN + GLOBE) {
            a -= GLOBE;
        }
        while (a < MIN) {
            a += GLOBE;
        }
        return a;
    }
    const double GLOBE;
    const double MIN;
};

template <int SCALE>
double convert_to_angle(complex_t) {
    NOTIMP; /* ensure specialisation */
}

template <int SCALE>
complex_t convert_to_complex(double) {
    NOTIMP; /* ensure specialisation */
}


// Specialised types

template <>
NormaliseAngle<DEGREE, ASYMMETRIC>::NormaliseAngle() :
    GLOBE(LongitudeDouble::GLOBE.value()), MIN(LongitudeDouble::GREENWICH.value()) {}
template <>
NormaliseAngle<DEGREE, SYMMETRIC>::NormaliseAngle() :
    GLOBE(LongitudeDouble::GLOBE.value()), MIN(LongitudeDouble::MINUS_DATE_LINE.value()) {}
template <>
NormaliseAngle<RADIAN, ASYMMETRIC>::NormaliseAngle() : GLOBE(M_PI * 2.), MIN(0.) {}
template <>
NormaliseAngle<RADIAN, SYMMETRIC>::NormaliseAngle() : GLOBE(M_PI * 2.), MIN(-M_PI) {}

template <>
double convert_to_angle<RADIAN>(complex_t c) {
    if (eckit::types::is_approximately_equal(std::real(c), 0.) &&
        eckit::types::is_approximately_equal(std::imag(c), 0.)) {
        return 0.;
    }
    return std::arg(c);
}

template <>
complex_t convert_to_complex<RADIAN>(double a) {
    return std::polar(1., a);
}

template <>
double convert_to_angle<DEGREE>(complex_t c) {
    return util::radian_to_degree(convert_to_angle<RADIAN>(c));
}

template <>
complex_t convert_to_complex<DEGREE>(double a) {
    return convert_to_complex<RADIAN>(util::degree_to_radian(a));
}


}  // namespace


static SpaceChoice<Space1DAngleT<DEGREE, ASYMMETRIC> > __space1("1d-angle-degree-asymmetric");
static SpaceChoice<Space1DAngleT<DEGREE, SYMMETRIC> > __space2("1d-angle-degree-symmetric");
static SpaceChoice<Space1DAngleT<RADIAN, ASYMMETRIC> > __space3("1d-angle-radian-asymmetric");
static SpaceChoice<Space1DAngleT<RADIAN, SYMMETRIC> > __space4("1d-angle-radian-symmetric");


template <int SCALE, int SYMMETRY>
Space1DAngleT<SCALE, SYMMETRY>::Space1DAngleT() : Space() {}


template <int SCALE, int SYMMETRY>
void Space1DAngleT<SCALE, SYMMETRY>::linearise(const Space::Matrix& matrixIn, Space::Matrix& matrixOut,
                                               double missingValue) const {
    ASSERT(matrixIn.cols() == 1);
    matrixOut.resize(matrixIn.rows(), 2);  // allocates memory, not initialised

    complex_t xy;
    for (Matrix::Size i = 0; i < matrixIn.size(); ++i) {
        if (matrixIn(i, 0) == missingValue) {
            matrixOut(i, 0) = matrixOut(i, 1) = missingValue;
        }
        else {
            xy              = convert_to_complex<SCALE>(matrixIn[i]);
            matrixOut(i, 0) = xy.real();
            matrixOut(i, 1) = xy.imag();
        }
    }
}


template <int SCALE, int SYMMETRY>
void Space1DAngleT<SCALE, SYMMETRY>::unlinearise(const Space::Matrix& matrixIn, Space::Matrix& matrixOut,
                                                 double missingValue) const {
    ASSERT(matrixIn.rows() == matrixOut.rows());
    ASSERT(matrixIn.cols() == 2);
    ASSERT(matrixOut.cols() == 1);

    NormaliseAngle<SCALE, SYMMETRY> norm;

    complex_t xy;
    double th;
    for (Matrix::Size i = 0; i < matrixIn.rows(); ++i) {
        if (matrixIn(i, 0) == missingValue || matrixIn(i, 1) == missingValue) {
            matrixOut[i] = missingValue;
        }
        else {
            xy           = complex_t(matrixIn(i, 0), matrixIn(i, 1));
            th           = convert_to_angle<SCALE>(xy);
            matrixOut[i] = norm.normalise(th);
            if (matrixOut[i] > LongitudeDouble::GLOBE.value()) {
                th = 2.;
            }
        }
    }
}


template <int SCALE, int SYMMETRY>
size_t Space1DAngleT<SCALE, SYMMETRY>::dimensions() const {
    return 1;
}


}  // namespace space
}  // namespace data
}  // namespace mir
