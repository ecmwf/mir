/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_data_dimension_Dimension1DAngleT_h
#define mir_data_dimension_Dimension1DAngleT_h

#include "mir/data/Dimension.h"

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


template< int SCALE, int SYMMETRY >
class Dimension1DAngleT : public Dimension {
public:

    // -- Exceptions
    // None

    // -- Constructors

    Dimension1DAngleT();

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void linearise(const Dimension::Matrix&, Dimension::Matrix&, double missingValue) const;

    void unlinearise(const Dimension::Matrix&, Dimension::Matrix&, double missingValue) const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Types
    // None

    // -- Members
    // None

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


}  // namespace dimension
}  // namespace data
}  // namespace mir


#endif

